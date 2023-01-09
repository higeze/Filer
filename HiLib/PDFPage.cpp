#include "PDFPage.h"
#include "PDFPageCache.h"
#include "PDFDoc.h"
#include <mutex>
#include <boost/algorithm/string.hpp>
#include "async_catch.h"
#include "ThreadPool.h"
#include "PdfView.h"
#include "PDFViewport.h"
#include "MyClipboard.h"

#include "strconv.h"
#include <math.h>

namespace sml = boost::sml;

/************/
/* CPdfPage */
/************/
struct CPDFPage::MachineBase
{
	template<class TRect, class R, class E>
	auto call(R(TRect::* f)(E))const
	{
		return [f](TRect* self, E e, boost::sml::back::process<E> process) { return (self->*f)(e); };
	}
	
	template<class TRect, class R>
	auto call(R(TRect::* f)())const
	{
		return [f](TRect* self) { return (self->*f)(); };
	}
};


//struct CPDFPage::BitmapMachine: public CPDFPage::MachineBase
//{
//	//Status
//	class None {};
//	class Loading {};
//	class Available {};
//	//class WaitCancel {};
//	class Error {};
//
//	auto operator()() const noexcept
//	{
//		using namespace sml;
//		return make_transition_table(
//			*state<None> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_None_Render),
//			state<None> +event<InitialLoadEvent> = state<Loading>,
//
//			state<Loading> +on_entry<InitialLoadEvent> / call(&CPDFPage::Bitmap_InitialLoading_OnEntry),
//			state<Loading> +on_entry<BitmapReloadEvent> / [](CPDFPage* p, const BitmapReloadEvent& e) { return p->Bitmap_Loading_OnEntry(e.DirectPtr, e.Scale); },
//			state<Loading> +on_entry<BitmapLoadCompletedEvent> / [](CPDFPage* p, const BitmapLoadCompletedEvent& e) { return p->Bitmap_Loading_OnEntry(e.DirectPtr, e.Scale); },
//			state<Loading> +on_entry<BitmapCancelCompletedEvent> / [](CPDFPage* p, const BitmapCancelCompletedEvent& e) { return p->Bitmap_Loading_OnEntry(e.DirectPtr, e.Scale); },
//
//			state<Loading> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_Loading_Render),
//			state<Loading> +event<BitmapLoadCompletedEvent> = state<Available>,
//			state<Loading> +event<BitmapReloadEvent> = state<Loading>,//state<WaitCancel>,
//			state<Loading> +event<BitmapErrorEvent> = state<Error>,
//
//			state<Available> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_Available_Render),
//			state<Available> +event<RenderPageSelectedTextEvent> / call(&CPDFPage::Bitmap_Available_RenderSelectedText),
//			state<Available> +event<RenderPageCaretEvent> / call(&CPDFPage::Bitmap_Available_RenderCaret),
//			state<Available> +event<BitmapReloadEvent> = state<Loading>,
//
//			//state<WaitCancel> +on_entry<_> / call(&CPDFPage::Bitmap_WaitCancel_OnEntry),
//			//state<WaitCancel> +on_exit<_> / call(&CPDFPage::Bitmap_WaitCancel_OnExit),
//
//			//state<WaitCancel> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_WaitCancel_Render),
//			//state<WaitCancel> +event<BitmapLoadCompletedEvent> = state<Loading>,
//			//state<WaitCancel> +event<BitmapCancelCompletedEvent> = state<Loading>,
//
//			state<Error> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_Error_Render)
//
//			//Error handler
//			//*state<Error> +exception<std::exception> = state<Error>
//		);
//	}
//};

struct CPDFPage::FindMachine: public CPDFPage::MachineBase
{
	//Status
	class None {};
	class Loading {};
	class Available {};
	//class WaitCancel {};
	class Error {};

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<None> +event<RenderPageFindEvent> / call(&CPDFPage::Find_None_Render),
			state<None> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_None_RenderLine),
			state<None> +event<FindLoadEvent> = state<Loading>,

			state<Loading> +on_entry<FindLoadEvent> / call(&CPDFPage::Find_Loading_OnEntry),
			state<Loading> +on_entry<FindReloadEvent> / call(&CPDFPage::Find_Loading_OnReEntry),

			state<Loading> +event<RenderPageFindEvent> / call(&CPDFPage::Find_Loading_Render),
			state<Loading> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_Loading_RenderLine),
			state<Loading> +event<FindLoadCompletedEvent> = state<Available>,
			state<Loading> +event<FindReloadEvent> = state<Loading>,//state<WaitCancel>,
			state<Loading> +event<FindErrorEvent> = state<Error>,

			state<Available> +event<RenderPageFindEvent> / call(&CPDFPage::Find_Available_Render),
			state<Available> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_Available_RenderLine),
			state<Available> +event<FindReloadEvent> = state<Loading>,

			//state<WaitCancel> +on_entry<_> / call(&CPDFPage::Find_WaitCancel_OnEntry),
			//state<WaitCancel> +on_exit<_> / call(&CPDFPage::Find_WaitCancel_OnExit),

			//state<WaitCancel> +event<RenderPageFindEvent> / call(&CPDFPage::Find_WaitCancel_Render),
			//state<WaitCancel> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_WaitCancel_RenderLine),
			//state<WaitCancel> +event<FindLoadCompletedEvent> = state<Loading>,
			//state<WaitCancel> +event<FindCancelCompletedEvent> = state<Loading>,

			state<Error> +event<RenderPageFindEvent> / call(&CPDFPage::Find_Error_Render),
			state<Error> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_Error_RenderLine)
			//Error handler
			//*state<Error> +exception<std::exception> = state<Error>
		);
	}
};

//void CPDFPage::RenderContent(const RenderPageContentEvent& e) { process_event(e); }
//{ 
//	const CSizeF szf = GetSourceSize();
//	const CSize sz(static_cast<int>(std::round(szf.width * e.Scale)), static_cast<int>(std::round(szf.height * e.Scale)));
//	const CRectF rcInPdfium(e.ViewportPtr->PageToPdfiumPage(m_index, e.RectInPage));
//
//	//GetPDFiumPtr()->RenderPage(
//	//	e.DirectPtr->GetHDC(),
//	//	GetPagePtr().get()
//	//	lefttop.x,
//	//	lefttop.y,
//	//	bw, bh, 0,
//	//	FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
//
//	BITMAPINFOHEADER bmih
//	{
//		.biSize = sizeof(BITMAPINFOHEADER),
//		.biWidth = static_cast<int>(std::round(rcInPdfium.Width())),
//		.biHeight = - static_cast<int>(std::round(std::abs(rcInPdfium.Height()))),
//		.biPlanes = 1,
//		.biBitCount = 32,
//		.biCompression = BI_RGB,
//		.biSizeImage = 0,	
//	};
//	
//	void* bitmapBits = nullptr;
//
//	std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(
//		::CreateDIBSection(e.DirectPtr->GetHDC(), reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0)
//	);
//	FALSE_THROW(pBmp);
//
//	UNQ_FPDF_BITMAP pFpdfBmp(GetPDFiumPtr()->Bitmap_UnqCreateEx(bmih.biWidth, - bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3)));
//	FALSE_THROW(pFpdfBmp);
//
//	GetPDFiumPtr()->Bitmap_FillRect(pFpdfBmp.get(), 0, 0, bmih.biWidth, - bmih.biHeight, 0xFFFFFFFF); // Fill white
//	FS_MATRIX mat{ e.Scale, 0, 0, e.Scale,0,0 };
//	//FS_RECTF rcf{ rcInPdfium.left, rcInPdfium.top, rcInPdfium.right, rcInPdfium.bottom };
//	FS_RECTF rcf{ 0, 0, 400, 400 };
//	GetPDFiumPtr()->RenderPageBitmapWithMatrix(pFpdfBmp.get(), GetPagePtr().get(), &mat, &rcf, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
//
//	CComPtr<IWICBitmap> pWICBitmap;
//	FAILED_THROW(e.DirectPtr->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));
//	
//	CComPtr<ID2D1Bitmap> pBitmap;
//	FAILED_THROW(e.DirectPtr->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));
//
//	auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//	auto rc = CRectF(
//			std::round(ptInWnd.x),
//			std::round(ptInWnd.y),
//			std::round(ptInWnd.x) + bmih.biWidth,
//			std::round(ptInWnd.y) - bmih.biHeight);
//
//		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pBitmap, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
//
//	
//	//e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pBitmap, e.ViewportPtr->PdfiumPageToWnd(m_index, rcInPdfium), 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
//}
//void CPDFPage::RenderFind(const RenderPageFindEvent& e) { process_event(e); }
//void CPDFPage::RenderFindLine(const RenderPageFindLineEvent& e) { process_event(e); }
//void CPDFPage::RenderSelectedText(const RenderPageSelectedTextEvent& e) { process_event(e); }
//void CPDFPage::RenderCaret(const RenderPageCaretEvent& e) { process_event(e); }
//void CPDFPage::process_event(const RenderPageContentEvent& e) { m_pBitmapMachine->process_event(e); }
//void CPDFPage::process_event(const RenderPageFindEvent& e) { m_pFindMachine->process_event(e); }
//void CPDFPage::process_event(const RenderPageFindLineEvent& e) { m_pFindMachine->process_event(e); }
//void CPDFPage::process_event(const RenderPageSelectedTextEvent& e) { m_pBitmapMachine->process_event(e); }
//void CPDFPage::process_event(const RenderPageCaretEvent& e) { m_pBitmapMachine->process_event(e); }
//
//void CPDFPage::process_event(const InitialLoadEvent& e) { m_pBitmapMachine->process_event(e); }
//
//void CPDFPage::process_event(const BitmapReloadEvent& e) { m_pBitmapMachine->process_event(e); }
//void CPDFPage::process_event(const BitmapLoadCompletedEvent& e) { m_pBitmapMachine->process_event(e); }
//void CPDFPage::process_event(const BitmapCancelCompletedEvent& e) { m_pBitmapMachine->process_event(e); }
//void CPDFPage::process_event(const BitmapErrorEvent& e) { m_pBitmapMachine->process_event(e); }

void CPDFPage::process_event(const FindLoadEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindReloadEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindLoadCompletedEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindCancelCompletedEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindErrorEvent& e) { m_pFindMachine->process_event(e); }


CPDFPage::CPDFPage(CPDFDoc* pDoc, int index )
	:m_pDoc(pDoc), m_index(index),
	//m_spCancelBitmapThread(std::make_shared<bool>(false)),
	//m_spCancelFindThread(std::make_shared<bool>(false)),
	m_fnd(),
	//m_txt(),
	m_loadingScale(0.f),
	m_loadingRotate(0),
	//m_pBitmapMachine(new sml::sm<BitmapMachine, boost::sml::process_queue<std::queue>>{ this }),
	m_pFindMachine(new sml::sm<FindMachine, boost::sml::process_queue<std::queue>>{ this })
{

	m_pPage = std::move(GetPDFiumPtr()->UnqLoadPage(pDoc->GetDocPtr().get(), m_index));
	m_pTextPage = std::move(GetPDFiumPtr()->Text_UnqLoadPage(GetPagePtr().get()));

	Rotate.set(GetPDFiumPtr()->Page_GetRotation(m_pPage.get()));
	Rotate.Subscribe([this](const int& value) 
		{
			GetPDFiumPtr()->Page_SetRotation(m_pPage.get(), value);
			ClearSourceSize();
		
		},(std::numeric_limits<sigslot::group_id>::min)());
}

CPDFPage::~CPDFPage() = default;

std::unique_ptr<CPDFiumSingleThread>& CPDFPage::GetPDFiumPtr() { return m_pDoc->GetPDFiumPtr(); }

void CPDFPage::LoadSourceSize()
{
	m_optSourceSize = CSizeF();
	m_optSourceSize->width = static_cast<FLOAT>(GetPDFiumPtr()->GetPageWidth(GetPagePtr().get()));
	m_optSourceSize->height = static_cast<FLOAT>(GetPDFiumPtr()->GetPageHeight(GetPagePtr().get()));
}

CComPtr<ID2D1Bitmap1> CPDFPage::GetBitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int& rotate, const CRectU& rectInPage)
{
	const CSizeF sz = GetSourceSize();
	//const int bw = static_cast<int>(std::round(sz.width * scale)); // Bitmap width
	//const int bh = static_cast<int>(std::round(sz.height * scale)); // Bitmap height
	BITMAPINFOHEADER bmih
	{
		.biSize = sizeof(BITMAPINFOHEADER),
		.biWidth = static_cast<LONG>(rectInPage.Width()),
		.biHeight = - static_cast<LONG>(rectInPage.Height()),
		.biPlanes = 1,
		.biBitCount = 32,
		.biCompression = BI_RGB,
		.biSizeImage = 0,	
	};

	void* bitmapBits = nullptr;

	UHBITMAP pBmp(::CreateDIBSection(pDirect->GetHDC(), reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
	FALSE_THROW(pBmp);

	UNQ_FPDF_BITMAP pFpdfBmp(GetPDFiumPtr()->Bitmap_UnqCreateEx(bmih.biWidth, - bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3)));
	FALSE_THROW(pFpdfBmp);

	GetPDFiumPtr()->Bitmap_FillRect(pFpdfBmp.get(), 0, 0, bmih.biWidth, - bmih.biHeight, 0xFFFFFFFF); // Fill white
	FS_MATRIX mat{ scale, 0.f, 0.f, scale, -static_cast<float>(rectInPage.left), -static_cast<float>(rectInPage.top) };
	FS_RECTF rcf{ 0, 0, static_cast<float>(bmih.biWidth), static_cast<float>(- bmih.biHeight) };
	GetPDFiumPtr()->RenderPageBitmapWithMatrix(pFpdfBmp.get(), GetPagePtr().get(), &mat, &rcf, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);

	CComPtr<IWICBitmap> pWICBitmap;
	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));
	
	CComPtr<ID2D1Bitmap1> pBitmap;
	FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));

	return pBitmap;
}

CRectF CPDFPage::GetCaretRect(const int index)
{
	const auto txt = GetTextInfo();
	if (!txt.CRLFRects.empty()) {
		auto rcInPdfiumPage = txt.CRLFRects[index];
		switch (Rotate.get()) {
			case 0:
				rcInPdfiumPage.right = rcInPdfiumPage.left + 1.f;
				break;
			case 1:
				rcInPdfiumPage.bottom = rcInPdfiumPage.top + 1.f;
				break;
			case 2:
				rcInPdfiumPage.left = rcInPdfiumPage.right - 1.f;
				break;
			case 3:
				rcInPdfiumPage.top = rcInPdfiumPage.bottom - 1.f;
				break;
		}
		return rcInPdfiumPage;
	} else {
		CRectF();
	}
}


//
//void CPDFPage::LoadBitmap(CDirect2DWrite* pDirect, const FLOAT& scale, const int& rotate)
//{
//	m_loadingScale.set(scale);
//	m_loadingRotate.set(rotate);
//
//	const CSizeF sz = GetSourceSize();
//	const int bw = static_cast<int>(std::round(sz.width * scale)); // Bitmap width
//	const int bh = static_cast<int>(std::round(sz.height * scale)); // Bitmap height
//
//	BITMAPINFOHEADER bmih{};
//	bmih.biSize = sizeof(BITMAPINFOHEADER);
//	bmih.biWidth = bw;
//	bmih.biHeight = -bh;
//	bmih.biPlanes = 1; 
//	bmih.biBitCount = 32;
//	bmih.biCompression = BI_RGB;
//	bmih.biSizeImage = 0;
//	
//	void* bitmapBits = nullptr;
//
//	if (scale != m_loadingScale.get() || rotate != m_loadingRotate.get()) { return; }
//	std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(
//		::CreateDIBSection(pDirect->GetHDC(), reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0)
//	);
//	FALSE_THROW(pBmp);
//
//	if (scale != m_loadingScale.get() || rotate != m_loadingRotate.get()) { return; }
//	UNQ_FPDF_BITMAP pFpdfBmp(GetPDFiumPtr()->Bitmap_UnqCreateEx(bw, bh, FPDFBitmap_BGRx, bitmapBits, (bw * 4)));
//	FALSE_THROW(pFpdfBmp);
//
//	if (scale != m_loadingScale.get() || rotate != m_loadingRotate.get()) { return; }
//	GetPDFiumPtr()->Bitmap_FillRect(pFpdfBmp.get(), 0, 0, bw, bh, 0xFFFFFFFF); // Fill white
//	GetPDFiumPtr()->RenderPageBitmap(pFpdfBmp.get(), GetPagePtr().get(), 0, 0, bw, bh, 0, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
//
//	if (scale != m_loadingScale.get() || rotate != m_loadingRotate.get()) { return; }
//	CComPtr<IWICBitmap> pWICBitmap;
//	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));
//	
//	if (scale != m_loadingScale.get() || rotate != m_loadingRotate.get()) { return; }
//	CComPtr<ID2D1Bitmap> pBitmap;
//	FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));
//	
//	if (scale != m_loadingScale.get() || rotate != m_loadingRotate.get()) { return; }
//	m_pDoc->GetPDFPageCachePtr()->InsertOrAssign(m_index, PdfBmpInfo{ pBitmap, scale, rotate });
//}

void CPDFPage::LoadFind(const std::wstring& find_string)
{
	auto find = boost::trim_copy(find_string);
	auto rects = std::vector<CRectF>();
	if(find.empty())
	{
		m_fnd.set(PdfFndInfo(rects, find.c_str()));
		return;
	}

	FPDF_WIDESTRING text = reinterpret_cast<FPDF_WIDESTRING>(find.c_str());
	UNQ_FPDF_SCHHANDLE pSchHdl(GetPDFiumPtr()->Text_UnqFindStart(m_pTextPage.get(), text, 0, 0));
	while (GetPDFiumPtr()->Text_FindNext(pSchHdl.get())) {
		int index = GetPDFiumPtr()->Text_GetSchResultIndex(pSchHdl.get());
		int ch_count = GetPDFiumPtr()->Text_GetSchCount(pSchHdl.get());
		int rc_count = GetPDFiumPtr()->Text_CountRects(m_pTextPage.get(), index, ch_count);
		for (int i = 0; i < rc_count; i++) {
			double left, top, right, bottom;
			m_pDoc->GetPDFiumPtr()->Text_GetRect(
				m_pTextPage.get(), 
				i,
				&left, 
				&top, 
				&right,
				&bottom);
			rects.emplace_back(
				static_cast<FLOAT>(left), 
				static_cast<FLOAT>(top), 
				static_cast<FLOAT>(right), 
				static_cast<FLOAT>(bottom));
		}
	}
	m_fnd.set(PdfFndInfo(rects, find.c_str()));
}

CRectF CPDFPage::RotateRect(const CRectF& rc, const int& rotate)
{
	CSizeF size = GetSourceSize();
	const FLOAT pw = size.width;
	const FLOAT ph = size.height;


	auto RectTransform = [](const CRectF& rc, FLOAT a, FLOAT b, FLOAT c, FLOAT d, FLOAT tx, FLOAT ty)->CRectF
	{
		auto PointTransform = [](const CPointF& pt, FLOAT a, FLOAT b, FLOAT c, FLOAT d, FLOAT tx, FLOAT ty)->CPointF
		{
			return CPointF(((a * pt.x) + (c * pt.y) + tx), ((b * pt.x) + (d * pt.y) + ty));
		};
		CPointF pt1(rc.left, rc.top); pt1 = PointTransform(pt1, a, b, c, d, tx, ty);
		CPointF pt2(rc.right, rc.bottom); pt2 = PointTransform(pt2, a, b, c, d, tx, ty);

		if (pt1.x > pt2.x) { std::swap(pt1.x, pt2.x); }
		if (pt1.y < pt2.y) { std::swap(pt1.y, pt2.y); }

		return CRectF(pt1.x, pt1.y, pt2.x, pt2.y);
	};

	switch (rotate) {
		default:
		case 0: // 0 degrees
		{
			//const UXMatrix t = { 1.0, 0.0, 0.0, -1.0, 0.0, ph };
			return RectTransform(rc, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
			break;
		}

		case 1: // 90 degrees
		{
			//const UXMatrix t = { 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			return  RectTransform(rc, 0.f, -1.f, 1.f, 0.f, 0.f, ph);
			break;
		}

		case 2: // 180 degrees
		{
			//const UXMatrix t = { -1.0, 0.0, 0.0, 1.0, pw, 0.0 };
			return RectTransform(rc, -1.f, 0.f, 0.f, -1.f, pw, ph);
			break;
		}

		case 3: // 270 degrees
		{
			//const UXMatrix t = { 0.0, -1.0, -1.0, 0.0, pw, ph };
			return RectTransform(rc, 0.f, 1.f, -1.f, 0.f, pw, 0.f);
			break;
		}
	}
}

void CPDFPage::RotateRects(std::vector<CRectF>& rects, const int& rotate)
{
	for (CRectF& rc : rects) {
		rc = RotateRect(rc, rotate);
	}
}

PdfTxtInfo CPDFPage::GetTextInfo()
{
	auto isCRorLF = [](const wchar_t ch)->bool { return ch == L'\r' || ch == L'\n'; };
	auto is_space = [](const wchar_t ch)->bool { return ch == L' ' || ch == L'@'; };
	auto is_cr = [](const wchar_t ch)->bool { return ch == L'\r'; };
	auto is_lf = [](const wchar_t ch)->bool { return ch == L'\n'; };
	int char_count = GetPDFiumPtr()->Text_CountChars(m_pTextPage.get());
	//Str
	std::wstring text(char_count, 0);
	int text_count = GetPDFiumPtr()->Text_GetText(m_pTextPage.get(), 0, char_count, reinterpret_cast<unsigned short*>(text.data()));
	//::ReleaseBuffer(text);

	//Original Char Rects
	auto orgCharRects = std::vector<CRectF>();
	for (auto i = 0; i < char_count; i++) {
		int rect_count = GetPDFiumPtr()->Text_CountRects(m_pTextPage.get(), i, 1);
		if (rect_count == 1) {
			double left, top, right, bottom = 0.f;
			m_pDoc->GetPDFiumPtr()->Text_GetRect(
				m_pTextPage.get(),
				0,
				&left,
				&top,
				&right,
				&bottom);
			orgCharRects.emplace_back(
				static_cast<FLOAT>(left),
				static_cast<FLOAT>(top),
				static_cast<FLOAT>(right),
				static_cast<FLOAT>(bottom));
		} else {
			auto stop = true;
		}
	}

	if (!orgCharRects.empty()) {
		//Cursor Rects
		auto cursorRects = orgCharRects;
		for (std::size_t i = 1; i < cursorRects.size(); i++) {//If i = 0  is Space or CRLF, ignore
			bool isFirst = i == 0;
			bool isAfterLF = !isFirst && is_lf(text[i - 1]);
			if (is_space(text[i])) {
				if (isFirst || isAfterLF) {
					cursorRects[i].SetRect(
						cursorRects[i + 1].right,
						cursorRects[i + 1].top,
						cursorRects[i + 1].right,
						cursorRects[i + 1].bottom
					);
				} else {
					cursorRects[i].SetRect(
						cursorRects[i - 1].right,
						cursorRects[i - 1].top,
						cursorRects[i - 1].right,
						cursorRects[i - 1].bottom
					);
				}
			}
			if (isCRorLF(text[i])) {
				cursorRects[i].SetRect(
					cursorRects[i - 1].right,
					cursorRects[i - 1].top,
					cursorRects[i - 1].right,
					cursorRects[i - 1].bottom
				);
			}
		}
		cursorRects.emplace_back(
			cursorRects.back().right, cursorRects.back().top,
			cursorRects.back().right, cursorRects.back().bottom);

		//Mouse Rects
		auto mouseRects = cursorRects;

		for (std::size_t i = 0; i < mouseRects.size(); i++) {
			bool isFirst = i == 0;
			bool isLast = i == mouseRects.size() - 1;
			bool isAfterLF = !isFirst && is_lf(text[i - 1]);
			bool isBeforeLast = i == mouseRects.size() - 2;
			bool isBeforeCR = !isLast && !isBeforeLast && is_cr(text[i + 1]);
			bool isAfterSpace = !isFirst && is_space(text[i - 1]);
			bool isBeforeSpace = !isLast && !isBeforeLast && is_space(text[i + 1]);
			bool isSpaceMid = !isFirst && is_space(text[i]);
			bool isCR = text[i] == L'\r';
			bool isLF = text[i] == L'\n';

			//left & right
			if (isCR || isSpaceMid || isLast) {
				mouseRects[i].left = mouseRects[i - 1].right;
				mouseRects[i].right = cursorRects[i - 1].right + cursorRects[i - 1].Width() * 0.5f;
			} else if (isLF) {
				mouseRects[i].left = mouseRects[i - 1].left;
				mouseRects[i].right = mouseRects[i - 1].right;
			} else {
				//left
				if (isFirst || isAfterLF || isAfterSpace) {
					mouseRects[i].left = (std::max)(cursorRects[i].left - cursorRects[i].Width() * 0.5f, 0.f);
				} else {
					mouseRects[i].left = (std::max)(cursorRects[i].left - cursorRects[i].Width() * 0.5f, mouseRects[i - 1].right);
				}
				//right
				if (isBeforeCR || isBeforeSpace || isBeforeLast) {
					mouseRects[i].right = cursorRects[i].right - cursorRects[i].Width() * 0.5f;
				} else {
					mouseRects[i].right = (std::min)(cursorRects[i].right + cursorRects[i].Width() * 0.5f, (cursorRects[i].right + cursorRects[i + 1].left) * 0.5f);
				}
			}
			//top & bottom
			//Since line order is not always top to bottom, it's hard to adjust context.
			mouseRects[i].top = cursorRects[i].top + (-cursorRects[i].Height()) * 0.2f;
			mouseRects[i].bottom = (std::max)(cursorRects[i].bottom - (-cursorRects[i].Height()) * 0.2f, 0.f);
		}
		RotateRects(cursorRects, Rotate.get());
		RotateRects(mouseRects, Rotate.get());

		return PdfTxtInfo(orgCharRects, cursorRects, mouseRects, text);
	} else {
		return PdfTxtInfo(std::vector<CRectF>(), std::vector<CRectF>(), std::vector<CRectF>(), text);
	}
}

std::vector<CRectF> CPDFPage::GetSelectedTextRects(const int& begin, const int& end)
{
	int rect_count = m_pDoc->GetPDFiumPtr()->Text_CountRects(m_pTextPage.get(), e.SelectedBegin, e.SelectedEnd - e.SelectedBegin);
	for (auto i = 0; i < rect_count; i++) {
		double left, top, right, bottom;
		m_pDoc->GetPDFiumPtr()->Text_GetRect(
			m_pTextPage.get(),
			i,
			&left,
			&top,
			&right,
			&bottom);
		auto rcInPdfiumPage = RotateRect(
			CRectF(
				static_cast<FLOAT>(left),
				static_cast<FLOAT>(top),
				static_cast<FLOAT>(right),
				static_cast<FLOAT>(bottom)),
			Rotate.get());
		auto rcSelectInWnd = e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, rcInPdfiumPage);

		e.DirectPtr->FillSolidRectangle(
				*(m_pDoc->GetPropPtr()->SelectedFill), rcSelectInWnd);
	}
}

//void CPDFPage::LoadText()
//{
//	auto isCRorLF = [](const wchar_t ch)->bool { return ch == L'\r' || ch == L'\n'; };
//	auto is_space = [](const wchar_t ch)->bool { return ch == L' ' || ch == L'@'; };
//	auto is_cr = [](const wchar_t ch)->bool { return ch == L'\r'; };
//	auto is_lf = [](const wchar_t ch)->bool { return ch == L'\n'; };
//	int char_count = GetPDFiumPtr()->Text_CountChars(m_pTextPage.get());
//	//Str
//	std::wstring text(char_count, 0);
//	int text_count = GetPDFiumPtr()->Text_GetText(m_pTextPage.get(), 0, char_count, reinterpret_cast<unsigned short*>(text.data()));
//	//::ReleaseBuffer(text);
//
//	//Original Char Rects
//	auto orgCharRects = std::vector<CRectF>();
//	for (auto i = 0; i < char_count; i++) {
//		int rect_count = GetPDFiumPtr()->Text_CountRects(m_pTextPage.get(), i, 1);
//		if (rect_count == 1) {
//			double left, top, right, bottom = 0.f;
//			m_pDoc->GetPDFiumPtr()->Text_GetRect(
//				m_pTextPage.get(),
//				0,
//				&left,
//				&top,
//				&right,
//				&bottom);
//			orgCharRects.emplace_back(
//				static_cast<FLOAT>(left),
//				static_cast<FLOAT>(top),
//				static_cast<FLOAT>(right),
//				static_cast<FLOAT>(bottom));
//		} else {
//			auto stop = true;
//		}
//	}
//
//	if (!orgCharRects.empty()) {
//		//Cursor Rects
//		auto cursorRects = orgCharRects;
//		for (std::size_t i = 1; i < cursorRects.size(); i++) {//If i = 0  is Space or CRLF, ignore
//			bool isFirst = i == 0;
//			bool isAfterLF = !isFirst && is_lf(text[i - 1]);
//			if (is_space(text[i])) {
//				if (isFirst || isAfterLF) {
//					cursorRects[i].SetRect(
//						cursorRects[i + 1].right,
//						cursorRects[i + 1].top,
//						cursorRects[i + 1].right,
//						cursorRects[i + 1].bottom
//					);
//				} else {
//					cursorRects[i].SetRect(
//						cursorRects[i - 1].right,
//						cursorRects[i - 1].top,
//						cursorRects[i - 1].right,
//						cursorRects[i - 1].bottom
//					);
//				}
//			}
//			if (isCRorLF(text[i])) {
//				cursorRects[i].SetRect(
//					cursorRects[i - 1].right,
//					cursorRects[i - 1].top,
//					cursorRects[i - 1].right,
//					cursorRects[i - 1].bottom
//				);
//			}
//		}
//		cursorRects.emplace_back(
//			cursorRects.back().right, cursorRects.back().top,
//			cursorRects.back().right, cursorRects.back().bottom);
//
//		//Mouse Rects
//		auto mouseRects = cursorRects;
//
//		for (std::size_t i = 0; i < mouseRects.size(); i++) {
//			bool isFirst = i == 0;
//			bool isLast = i == mouseRects.size() - 1;
//			bool isAfterLF = !isFirst && is_lf(text[i - 1]);
//			bool isBeforeLast = i == mouseRects.size() - 2;
//			bool isBeforeCR = !isLast && !isBeforeLast && is_cr(text[i + 1]);
//			bool isAfterSpace = !isFirst && is_space(text[i - 1]);
//			bool isBeforeSpace = !isLast && !isBeforeLast && is_space(text[i + 1]);
//			bool isSpaceMid = !isFirst && is_space(text[i]);
//			bool isCR = text[i] == L'\r';
//			bool isLF = text[i] == L'\n';
//
//			//left & right
//			if (isCR || isSpaceMid || isLast) {
//				mouseRects[i].left = mouseRects[i - 1].right;
//				mouseRects[i].right = cursorRects[i - 1].right + cursorRects[i - 1].Width() * 0.5f;
//			} else if (isLF) {
//				mouseRects[i].left = mouseRects[i - 1].left;
//				mouseRects[i].right = mouseRects[i - 1].right;
//			} else {
//				//left
//				if (isFirst || isAfterLF || isAfterSpace) {
//					mouseRects[i].left = (std::max)(cursorRects[i].left - cursorRects[i].Width() * 0.5f, 0.f);
//				} else {
//					mouseRects[i].left = (std::max)(cursorRects[i].left - cursorRects[i].Width() * 0.5f, mouseRects[i - 1].right);
//				}
//				//right
//				if (isBeforeCR || isBeforeSpace || isBeforeLast) {
//					mouseRects[i].right = cursorRects[i].right - cursorRects[i].Width() * 0.5f;
//				} else {
//					mouseRects[i].right = (std::min)(cursorRects[i].right + cursorRects[i].Width() * 0.5f, (cursorRects[i].right + cursorRects[i + 1].left) * 0.5f);
//				}
//			}
//			//top & bottom
//			//Since line order is not always top to bottom, it's hard to adjust context.
//			mouseRects[i].top = cursorRects[i].top + (-cursorRects[i].Height()) * 0.2f;
//			mouseRects[i].bottom = (std::max)(cursorRects[i].bottom - (-cursorRects[i].Height()) * 0.2f, 0.f);
//		}
//		RotateRects(cursorRects, Rotate.get());
//		RotateRects(mouseRects, Rotate.get());
//
//		m_txt.set(PdfTxtInfo(orgCharRects, cursorRects, mouseRects, text));
//	} else {
//		m_txt.set(PdfTxtInfo(std::vector<CRectF>(), std::vector<CRectF>(), std::vector<CRectF>(), text));
//	}
//}

int CPDFPage::GetCursorCharIndexAtPos(const CPointF& ptInPdfiumPage)
{
	const auto txt = GetTextInfo();
	auto iter = std::find_if(txt.MouseRects.cbegin(), txt.MouseRects.cend(),
		[ptInPdfiumPage](const CRectF& rc)->bool {
			return rc.left <= ptInPdfiumPage.x && ptInPdfiumPage.x < rc.right &&
				rc.top >= ptInPdfiumPage.y && ptInPdfiumPage.y > rc.bottom;
		});
	if (iter == txt.MouseRects.cend()) {
		return -1;
	} else {
		return std::distance(txt.MouseRects.cbegin(), iter);
	}
}

CRectF CPDFPage::GetCursorRect(const int& index)
{
	return GetTextInfo().CRLFRects[index];
}

int CPDFPage::GetTextSize()
{
	return GetTextInfo().String.size();
}
std::wstring CPDFPage::GetText()
{
	return GetTextInfo().String;	
}

/**************/
/* SML Action */
/**************/
//void CPDFPage::Bitmap_None_Render(const RenderPageContentEvent& e)
//{
//	process_event(InitialLoadEvent(e.DirectPtr, e.Scale));
//}
//void CPDFPage::Bitmap_InitialLoading_OnEntry(const InitialLoadEvent& e) 
//{
//	auto fun = [this, e]()
//	{ 
//		this->LoadBitmap(e.DirectPtr, e.Scale, Rotate.get());
//		LoadText();
//		process_event(BitmapLoadCompletedEvent(e.DirectPtr, e.Scale));
//	};
//	
//	m_futureBitmap = GetPDFiumPtr()->GetThreadPtr()->enqueue(fun);
//}
//void CPDFPage::Bitmap_Loading_OnEntry(CDirect2DWrite* pDirect, const FLOAT& scale) 
//{
//	auto fun = [this, pDirect, scale]()
//	{ 
//		this->LoadBitmap(pDirect, scale, Rotate.get());
//		LoadText();
//		process_event(BitmapLoadCompletedEvent(pDirect, scale));
//	};
//	
//	m_futureBitmap = GetPDFiumPtr()->GetThreadPtr()->enqueue(fun);
//}
//
//void CPDFPage::Bitmap_Loading_Render(const RenderPageContentEvent& e) 
//{
//	auto iter = m_pDoc->GetPDFPageCachePtr()->Find(m_index);
//
//	if (iter == m_pDoc->GetPDFPageCachePtr()->CEnd() || e.Scale != m_loadingScale.get() || Rotate.get() != m_loadingRotate.get()) {
//		process_event(BitmapReloadEvent(e.DirectPtr, e.Scale));
//	}
//
//	if (iter != m_pDoc->GetPDFPageCachePtr()->CEnd() && Rotate.get() == m_loadingRotate.get()) {
//		auto sz = iter->second.BitmapPtr->GetSize();
//		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//		auto rc = CRectF(
//				std::round(ptInWnd.x),
//				std::round(ptInWnd.y),
//				std::round(ptInWnd.x + sz.width * e.Scale / iter->second.Scale),
//				std::round(ptInWnd.y + sz.height * e.Scale / iter->second.Scale));
//
//		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(iter->second.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
//		m_pDoc->GetPDFPageCachePtr()->Prune();
//
//	} else {
//		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//		e.DirectPtr->DrawTextFromPoint(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", ptInWnd);
//	}
//}
//
//void CPDFPage::Bitmap_Available_Render(const RenderPageContentEvent& e) 
//{
//	auto iter = m_pDoc->GetPDFPageCachePtr()->Find(m_index);
//
//	if (iter == m_pDoc->GetPDFPageCachePtr()->CEnd() || e.Scale != m_loadingScale.get() || Rotate.get() != m_loadingRotate.get()) {
//		process_event(BitmapReloadEvent(e.DirectPtr, e.Scale));
//	}
//
//	if (iter != m_pDoc->GetPDFPageCachePtr()->CEnd() && Rotate.get() == m_loadingRotate.get()) {
//		auto sz = iter->second.BitmapPtr->GetSize();
//		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//		auto rc = CRectF(
//				std::round(ptInWnd.x),
//				std::round(ptInWnd.y),
//				std::round(ptInWnd.x + sz.width * e.Scale / iter->second.Scale),
//				std::round(ptInWnd.y + sz.height * e.Scale / iter->second.Scale));
//
//		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(iter->second.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
//		m_pDoc->GetPDFPageCachePtr()->Prune();
//
//	} else {
//		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//		e.DirectPtr->DrawTextFromPoint(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", ptInWnd);
//	}
//}
//
//void CPDFPage::Bitmap_Available_RenderSelectedText(const RenderPageSelectedTextEvent& e)
//{
//	int rect_count = m_pDoc->GetPDFiumPtr()->Text_CountRects(m_pTextPage.get(), e.SelectedBegin, e.SelectedEnd - e.SelectedBegin);
//	for (auto i = 0; i < rect_count; i++) {
//		double left, top, right, bottom;
//		m_pDoc->GetPDFiumPtr()->Text_GetRect(
//			m_pTextPage.get(),
//			i,
//			&left,
//			&top,
//			&right,
//			&bottom);
//		auto rcInPdfiumPage = RotateRect(
//			CRectF(
//				static_cast<FLOAT>(left),
//				static_cast<FLOAT>(top),
//				static_cast<FLOAT>(right),
//				static_cast<FLOAT>(bottom)),
//			Rotate.get());
//		auto rcSelectInWnd = e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, rcInPdfiumPage);
//
//		e.DirectPtr->FillSolidRectangle(
//				*(m_pDoc->GetPropPtr()->SelectedFill), rcSelectInWnd);
//	}
//
//	if (e.Debug) {
//		const auto& txt = m_txt.get();
//		for (const auto& rc : txt.MouseRects) {
//			e.DirectPtr->DrawSolidRectangle(SolidLine(1.f, 0.f, 0.f, 1.f, 1.f), e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, rc));
//		}
//	}
//}
//
//void CPDFPage::Bitmap_Available_RenderCaret(const RenderPageCaretEvent& e)
//{
//	const auto& txt = m_txt.get();
//	if (!txt.CRLFRects.empty()) {
//		auto rcInPdfiumPage = txt.CRLFRects[e.CharIndex];
//		switch (Rotate.get()) {
//			case 0:
//				rcInPdfiumPage.right = rcInPdfiumPage.left + 1.f;
//				break;
//			case 1:
//				rcInPdfiumPage.bottom = rcInPdfiumPage.top + 1.f;
//				break;
//			case 2:
//				rcInPdfiumPage.left = rcInPdfiumPage.right - 1.f;
//				break;
//			case 3:
//				rcInPdfiumPage.top = rcInPdfiumPage.bottom - 1.f;
//				break;
//		}
//
//		e.DirectPtr->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
//		e.DirectPtr->FillSolidRectangle(m_pDoc->GetPropPtr()->Format->Color, e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, rcInPdfiumPage));
//		e.DirectPtr->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
//	}
//}

//void CPDFPage::Bitmap_WaitCancel_OnEntry()
//{
//	*m_spCancelBitmapThread = true;
//}
//void CPDFPage::Bitmap_WaitCancel_OnExit()
//{
//	*m_spCancelBitmapThread = false;
//}
//
//void CPDFPage::Bitmap_WaitCancel_Render(const RenderPageContentEvent& e) 
//{
//	auto iter = m_pDoc->GetPDFPageCachePtr()->Find(m_index);
//
//	//if (iter == m_pDoc->GetPDFPageCachePtr()->CEnd() || e.Scale != m_loadingScale || Rotate.get() != m_loadingRotate) {
//	//	process_event(BitmapReloadEvent(e.DirectPtr, e.Scale));
//	//}
//
//	if (iter != m_pDoc->GetPDFPageCachePtr()->CEnd() && Rotate.get() == m_loadingRotate) {
//		auto sz = iter->second.BitmapPtr->GetSize();
//		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//		auto rc = CRectF(
//				std::round(ptInWnd.x),
//				std::round(ptInWnd.y),
//				std::round(ptInWnd.x + sz.width * e.Scale / iter->second.Scale),
//				std::round(ptInWnd.y + sz.height * e.Scale / iter->second.Scale));
//
//		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(iter->second.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
//		m_pDoc->GetPDFPageCachePtr()->Prune();
//
//	} else {
//		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//		e.DirectPtr->DrawTextFromPoint(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", ptInWnd);
//	}
//}

//void CPDFPage::Bitmap_Error_Render(const RenderPageContentEvent& e)
//{
//	auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
//	e.DirectPtr->DrawTextFromPoint(*(m_pDoc->GetPropPtr()->Format), L"Error on Page loading.", ptInWnd);
//}

/********/
/* Find */
/********/

void CPDFPage::Find_None_Render(const RenderPageFindEvent& e)
{
	process_event(FindLoadEvent{e.Find});
}

void CPDFPage::Find_None_RenderLine(const RenderPageFindLineEvent& e)
{
	process_event(FindLoadEvent{e.Find});
}

void CPDFPage::Find_Loading_OnEntry(const FindLoadEvent& e) 
{
	auto fun = [this, find = e.Find]()
	{ 
		LoadFind(find);
		process_event(FindLoadCompletedEvent());
	};
	
	m_futureFind = GetPDFiumPtr()->GetThreadPtr()->enqueue(fun);
}

void CPDFPage::Find_Loading_OnReEntry(const FindReloadEvent& e)
{
	Find_Loading_OnEntry(FindLoadEvent{ e.Find });
}

void CPDFPage::Find_Available_Render(const RenderPageFindEvent& e) 
{
	const auto& fnd = m_fnd.get();
	if (e.Find != fnd.Find) {
		process_event(FindReloadEvent{ e.Find });
	}
	if (!fnd.FindRects.empty()) {
		for (const auto& ch_rc : fnd.FindRects) {
			e.DirectPtr->FillSolidRectangle( //TODO Find rect is not proper
				*(m_pDoc->GetPropPtr()->FindHighliteFill), e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, ch_rc));
		}
	}
}

void CPDFPage::Find_Available_RenderLine(const RenderPageFindLineEvent& e)
{
	const auto& fnd = m_fnd.get();
	if (e.Find != fnd.Find) {
		process_event(FindReloadEvent{ e.Find });
	}

	if (!fnd.FindRects.empty()){
		CSizeF srcSize = GetSourceSize();
		SolidFill fill(*m_pDoc->GetPropPtr()->FindHighliteFill);
		fill.Color.a = 1.f;

		for (auto ch_rc : fnd.FindRects) {
			ch_rc = e.ViewportPtr->PdfiumPageToPage(e.PageIndex, ch_rc);
			auto rectHighlite = CRectF(
					e.Rect.left,
					e.Rect.top + e.Rect.Height() * ch_rc.top / srcSize.height,
					e.Rect.right,
					e.Rect.top + e.Rect.Height() * ch_rc.bottom / srcSize.height);
			if (rectHighlite.Height() < 1.f) {
				rectHighlite.bottom = rectHighlite.top + 1.f;
			}
			e.DirectPtr->FillSolidRectangle(fill, rectHighlite);
		}
	}
}

//void CPDFPage::Find_WaitCancel_OnEntry()
//{
//	//*m_spCancelFindThread = true;
//}
//void CPDFPage::Find_WaitCancel_OnExit()
//{
//	//*m_spCancelFindThread = false;
//}