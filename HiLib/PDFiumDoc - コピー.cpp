#include "PDFiumDoc.h"
#include "D2DWWindow.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
#include <mutex>
#include "async_catch.h"

#include "FileIsInUse.h"

#include "strconv.h"

namespace sml = boost::sml;

void CPDFiumDoc::Init()
{
	const FPDF_LIBRARY_CONFIG config {2, nullptr, nullptr, 0};
	FPDF_InitLibraryWithConfig(&config);
}

void CPDFiumDoc::Term()
{
	FPDF_DestroyLibrary();
}

CPDFiumDoc::CPDFiumDoc(const std::wstring& path, const std::wstring& password, CDirect2DWrite* pDirect, const std::shared_ptr<FormatF>& pFormat, std::function<void()> changed)
	:m_path(path),
	m_password(password),
	m_pDirect(pDirect),
	m_pFormat(pFormat),
	m_changed(changed)
{
	GetDocPtr = [pDoc = std::shared_ptr<std::remove_pointer_t<FPDF_DOCUMENT>>(), path, password, this]() mutable
		->std::shared_ptr<std::remove_pointer_t<FPDF_DOCUMENT>>&
	{
		if (!pDoc) {
			pDoc.reset(FPDF_LoadDocument(wide_to_utf8(path).c_str(), wide_to_utf8(password).c_str()), fpdf_closedocument());
		}
		return pDoc;
	};

	GetPageCount = [count = int(0), this]()mutable->int&
	{
		if (count == 0) {
			count = FPDF_GetPageCount(GetDocPtr().get());
		}
		return count;
	};

	GetPages = [pages = std::vector<std::shared_ptr<CPDFiumPage>>(), this]() mutable
		->std::vector<std::shared_ptr<CPDFiumPage>>&
	{
		if (pages.empty()) {
			for (auto i = 0; i < GetPageCount(); i++) {
				pages.push_back(std::make_shared<CPDFiumPage>(this, i));
			}
		}
		return pages;
	};

	GetSourceSize = [sz = CSizeF(), this]()mutable->CSizeF&
	{	
		if (sz.width == 0 || sz.height == 0) {
			for(const auto& pPage : GetPages()){
				sz.width = (std::max)(sz.width, pPage->GetSourceSize().width);
				sz.height += pPage->GetSourceSize().height;
			}
		}
		return sz;
	};

}

CPDFiumDoc::~CPDFiumDoc() = default;

/************/
/* CPdfPage */
/************/

CPDFiumPage::CPDFiumPage(CPDFiumDoc* pDoc, int index )
	:m_pDoc(pDoc), m_index(index)
{
	GetPagePtr = [pPage = std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>(), this]() mutable
		->std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>&
	{
		if (!pPage) {
			pPage = std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>(FPDF_LoadPage(m_pDoc->GetDocPtr().get(), m_index), FPDF_ClosePage);
		}
		return pPage;
	};

	GetSourceSize = [sz = CSizeF(), this]()mutable->CSizeF
	{		
		if (sz.width == 0 || sz.height == 0) {
			sz.width = static_cast<FLOAT>(FPDF_GetPageWidth(GetPagePtr().get()));
			sz.height = static_cast<FLOAT>(FPDF_GetPageHeight(GetPagePtr().get()));
		}
		return sz;
	};

	GetBitmapPtr = [pBitmap = CComPtr<ID2D1Bitmap>(), cur_scale = 0.f,  this](FLOAT req_scale) mutable
		->CComPtr<ID2D1Bitmap>&
	{
		if (!pBitmap || cur_scale != req_scale) {
			cur_scale = req_scale;
			const CSizeF sz = GetSourceSize();
			const int bw = static_cast<int>(sz.width * req_scale + 0.5f); // Bitmap width
			const int bh = static_cast<int>(sz.height * req_scale + 0.5f); // Bitmap height

			BITMAPINFOHEADER bmih; RtlSecureZeroMemory(&bmih, sizeof(bmih));
			bmih.biSize = sizeof(bmih); bmih.biWidth = bw; bmih.biHeight = -bh;
			bmih.biPlanes = 1; bmih.biBitCount = 32; bmih.biCompression = BI_RGB;
			bmih.biSizeImage = (bw * bh * 4); 
	
			void* bitmapBits = nullptr;

			std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(
				::CreateDIBSection(m_pDoc->GetDirectPtr()->GetHDC(), reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0)
			);
			FALSE_THROW(pBmp);

			std::unique_ptr<std::remove_pointer_t<FPDF_BITMAP>, fpdfbitmap_destroy> pFpdfBmp(
				FPDFBitmap_CreateEx(bw, bh, FPDFBitmap_BGRx, bitmapBits, (bw * 4))
			);
			FALSE_THROW(pFpdfBmp);

			FPDFBitmap_FillRect(pFpdfBmp.get(), 0, 0, bw, bh, 0xFFFFFFFF); // Fill white

			const int options = (FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
			FPDF_RenderPageBitmap(pFpdfBmp.get(), GetPagePtr().get(), 0, 0, bw, bh, 0, options);

			CComPtr<IWICBitmap> pWICBitmap;
			FAILED_THROW(m_pDoc->GetDirectPtr()->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));

			pBitmap = CComPtr<ID2D1Bitmap>();
			FAILED_THROW(m_pDoc->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));
		}
		return pBitmap;
	};
}

CPDFiumPage::~CPDFiumPage() = default;

void CPDFiumPage::Render(const RenderEvent& e)
{
	//::OutputDebugString(std::format(L"Wnd:{},{}\r\n", e.RenderRectInWnd.Width(), e.RenderRectInWnd.Height()).c_str());
	//::OutputDebugString(std::format(L"Src:{},{}\r\n", GetSourceSize().width, GetSourceSize().height).c_str());
	//const CSizeF sz = GetSourceSize();
	//const int bw = static_cast<int>(sz.width * 1.f + 0.5f); // Bitmap width
	//const int bh = static_cast<int>(sz.height * 1.f + 0.5f); // Bitmap height
	//::OutputDebugString(std::format(L"SrcInt:{},{}\r\n", bw, bh).c_str());
	auto bmpRect = CRectF(
		std::round(e.RenderRectInWnd.left),
		std::round(e.RenderRectInWnd.top),
		std::round(e.RenderRectInWnd.right),
		std::round(e.RenderRectInWnd.bottom));
	//::OutputDebugString(std::format(L"DrawInt:{},{}\r\n", bw, bh).c_str());
	//auto pxsz = GetBitmapPtr(e.Scale)->GetPixelSize();
	//auto dpisz = GetBitmapPtr(e.Scale)->GetPixelSize();

	e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(GetBitmapPtr(e.Scale), bmpRect,1.f,D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
}

//
//void CPDFiumPage::Load()
//{
//	auto scale = m_requestingScale;
//	m_loadingScale = scale;
//
//	const CSizeF sz = GetSourceSize();
//	const int bw = static_cast<int>(sz.width * scale + 0.5f); // Bitmap width
//	const int bh = static_cast<int>(sz.height * scale + 0.5f); // Bitmap height
//
//	BITMAPINFOHEADER bmih; RtlSecureZeroMemory(&bmih, sizeof(bmih));
//	bmih.biSize = sizeof(bmih); bmih.biWidth = bw; bmih.biHeight = -bh;
//	bmih.biPlanes = 1; bmih.biBitCount = 32; bmih.biCompression = BI_RGB;
//	bmih.biSizeImage = (bw * bh * 4); 
//	
//	void* bitmapBits = nullptr;
//
//	std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(
//		::CreateDIBSection(m_pDoc->GetDirectPtr()->GetHDC(), reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0)
//	);
//	FALSE_THROW(pBmp);
//
//	std::unique_ptr<std::remove_pointer_t<FPDF_BITMAP>, fpdfbitmap_destroy> pFpdfBmp(
//		FPDFBitmap_CreateEx(bw, bh, FPDFBitmap_BGRx, bitmapBits, (bw * 4))
//	);
//	FALSE_THROW(pFpdfBmp);
//
//	FPDFBitmap_FillRect(pFpdfBmp.get(), 0, 0, bw, bh, 0xFFFFFFFF); // Fill white
//	//const FS_RECTF clip{ 0.0f, 0.0f, float(bw), float(bh) }; // To bitmap
//
//	const int options = (FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
//	FPDF_RenderPageBitmap(pFpdfBmp.get(), GetPagePtr().get(), 0, 0, bw, bh, 0, options);
//
//	CComPtr<IWICBitmap> pWICBitmap;
//	FAILED_THROW(m_pDoc->GetDirectPtr()->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));
//
//	CComPtr<ID2D1Bitmap> pBitmap;
//	FAILED_THROW(m_pDoc->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));
//
//	SetLockBitmap(PdfBmpInfo{ pBitmap, scale });
//}


/**************/
/* SML Action */
/**************/
//void CPDFiumPage::None_Render(const RenderEvent& e)
//{
//	m_requestingScale = e.Scale;
//	process_event(LoadEvent());
//}
//void CPDFiumPage::Loading_OnEntry() 
//{
//	auto fun = [this]()
//	{ 
//		Load();
//		process_event(LoadCompletedEvent());
//	};
//	//fun();
//	m_future = std::async(
//	std::launch::async,
//	async_action_wrap<decltype(fun)>,
//	fun);
//}
//
//void CPDFiumPage::Loading_Render(const RenderEvent& e) 
//{
//	m_requestingScale = e.Scale;
//
//	auto pbi = GetLockBitmap();
//	if (e.Scale != m_loadingScale) {
//		process_event(ReloadEvent());
//	}
//
//	if (pbi.BitmapPtr) {
//		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, e.RenderRectInWnd);
//	} else {
//		e.DirectPtr->DrawTextInRect(*m_pDoc->GetFormatPtr(), L"Loading Page...", e.RenderRectInWnd);
//	}
//}
//
//void CPDFiumPage::Available_Render(const RenderEvent& e) 
//{
//	m_requestingScale = e.Scale;
//
//	auto pbi = GetLockBitmap();
//	if (e.Scale != pbi.Scale) {
//		process_event(ReloadEvent());
//	}
//
//	if (pbi.BitmapPtr) {
//		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, e.RenderRectInWnd);
//	}
//}
//
//void CPDFiumPage::WaitCancel_OnEntry()
//{
//	*m_spCancelThread = true;
//}
//void CPDFiumPage::WaitCancel_OnExit()
//{
//	*m_spCancelThread = false;
//}
//
//void CPDFiumPage::WaitCancel_Render(const RenderEvent& e) 
//{
//	m_requestingScale = e.Scale;
//
//	auto pbi = GetLockBitmap();
//
//	if (pbi.BitmapPtr) {
//		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, e.RenderRectInWnd);
//	} else {
//		e.DirectPtr->DrawTextInRect(*m_pDoc->GetFormatPtr(), L"Loading Page...", e.RenderRectInWnd);
//	}
//}
//
//void CPDFiumPage::Error_Render(const RenderEvent& e)
//{
//	m_requestingScale = e.Scale;
//
//	e.DirectPtr->DrawTextInRect(*m_pDoc->GetFormatPtr(), L"Error on Page loading.", e.RenderRectInWnd);
//}
//
//
