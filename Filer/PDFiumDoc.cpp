#include "PDFiumDoc.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
#include <mutex>
#include "async_catch.h"


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

CPDFiumDoc::CPDFiumDoc(const std::wstring& path, const std::wstring& password, 
		CDirect2DWrite* pDirect, const std::shared_ptr<FormatF>& pFormat, std::function<void()> changed)
	:m_path(path),
	m_pDirect(pDirect),
	m_pFormat(pFormat),
	m_changed(changed)
{
	GetDocPtr = [pDoc = std::shared_ptr<std::remove_pointer_t<FPDF_DOCUMENT>>(), spMtx = std::make_shared<std::mutex>(), path, password, this]() mutable
		->std::shared_ptr<std::remove_pointer_t<FPDF_DOCUMENT>>&
	{
		std::lock_guard<std::mutex> lock(*spMtx);
		if (!pDoc) {
			pDoc.reset(FPDF_LoadDocument(wide_to_utf8(path).c_str(), wide_to_utf8(password).c_str()), FPDF_CloseDocument);
		}
		return pDoc;
	};

	GetPageCount = [count = int(0), spMtx = std::make_shared<std::mutex>(), this]()mutable->int
	{
		std::lock_guard<std::mutex> lock(*spMtx);
		if (count == 0) {
			count = FPDF_GetPageCount(GetDocPtr().get());
		}
		return count;
	};

	GetPages = [pages = std::vector<std::shared_ptr<CPDFiumPage>>(), spMtx = std::make_shared<std::mutex>(), this]() mutable
		->std::vector<std::shared_ptr<CPDFiumPage>>&
	{
		std::lock_guard<std::mutex> lock(*spMtx);
		if (pages.empty()) {
			for (auto i = 0; i < GetPageCount(); i++) {
				pages.push_back(std::make_shared<CPDFiumPage>(this, i));
			}
		}
		return pages;
	};

	GetSourceSize = [sz = CSizeF(), spMtx = std::make_shared<std::mutex>(), this]()mutable->CSizeF
	{	
		std::lock_guard<std::mutex> lock(*spMtx);
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

struct CPDFiumPage::Machine
{
	//Status
	class None {};
	class Loading {};
	class Available {};
	class WaitCancel {};
	class Error {};

	template<class T, class R, class E>
	auto call(R(T::* f)(E))const
	{
		return [f](T* self, E e, boost::sml::back::process<E> process) { return (self->*f)(e); };
	}
	
	template<class T, class R>
	auto call(R(T::* f)())const
	{
		return [f](T* self) { return (self->*f)(); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<None> +event<RenderEvent> / call(&CPDFiumPage::None_Render),
			state<None> +event<LoadEvent> = state<Loading>,

			state<Loading> +on_entry<_> / call(&CPDFiumPage::Loading_OnEntry),

			state<Loading> +event<RenderEvent> / call(&CPDFiumPage::Loading_Render),
			state<Loading> +event<LoadCompletedEvent> = state<Available>,
			state<Loading> +event<ReloadEvent> = state<WaitCancel>,
			state<Loading> +event<ErrorEvent> = state<Error>,

			state<Available> +event<RenderEvent> / call(&CPDFiumPage::Available_Render),
			state<Available> +event<ReloadEvent> = state<Loading>,

			state<WaitCancel> +on_entry<_> / call(&CPDFiumPage::WaitCancel_OnEntry),
			state<WaitCancel> +on_exit<_> / call(&CPDFiumPage::WaitCancel_OnExit),

			state<WaitCancel> +event<RenderEvent> / call(&CPDFiumPage::WaitCancel_Render),
			state<WaitCancel> +event<LoadCompletedEvent> = state<Loading>,
			state<WaitCancel> +event<CancelCompletedEvent> = state<Loading>,

			state<Error> +event<RenderEvent> / call(&CPDFiumPage::Error_Render)

			//Error handler
			//*state<Error> +exception<std::exception> = state<Error>
		);
	}
};

void CPDFiumPage::process_event(const RenderEvent& e) { m_pMachine->process_event(e); }
void CPDFiumPage::process_event(const LoadEvent& e) { m_pMachine->process_event(e); }
void CPDFiumPage::process_event(const ReloadEvent& e) { m_pMachine->process_event(e); }
void CPDFiumPage::process_event(const LoadCompletedEvent& e) { m_pMachine->process_event(e); }
void CPDFiumPage::process_event(const CancelCompletedEvent& e) { m_pMachine->process_event(e); }
void CPDFiumPage::process_event(const ErrorEvent& e) { m_pMachine->process_event(e); }


CPDFiumPage::CPDFiumPage(CPDFiumDoc* pDoc, int index )
	:m_pDoc(pDoc), m_index(index),
	m_spCancelThread(std::make_shared<bool>(false)),
	m_bmp{ CComPtr<ID2D1Bitmap>(), 0.f },
	m_loadingScale(0.f), m_requestingScale(0.f),
	m_pMachine(new sml::sm<Machine, boost::sml::process_queue<std::queue>>{ this })
{
	GetPagePtr = [pPage = std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>(), spMtx = std::make_shared<std::mutex>(), this]() mutable
		->std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>&
	{
		std::lock_guard<std::mutex> lock(*spMtx);
		if (!pPage) {
			pPage = std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>(FPDF_LoadPage(m_pDoc->GetDocPtr().get(), m_index), FPDF_ClosePage);
		}
		return pPage;
	};

	GetSourceSize = [sz = CSizeF(), spMtx = std::make_shared<std::mutex>(), this]()mutable->CSizeF
	{		
		std::lock_guard<std::mutex> lock(*spMtx);
		if (sz.width == 0 || sz.height == 0) {
			sz.width = static_cast<FLOAT>(FPDF_GetPageWidth(GetPagePtr().get()));
			sz.height = static_cast<FLOAT>(FPDF_GetPageHeight(GetPagePtr().get()));
		}
		return sz;
	};
}

CPDFiumPage::~CPDFiumPage()
{
	*m_spCancelThread = true;
	m_future.get();
}

void CPDFiumPage::Load()
{
	auto scale = m_requestingScale;
	m_loadingScale = scale;

	const CSizeF sz = GetSourceSize();
	const int bw = static_cast<int>(sz.width * scale + 0.5f); // Bitmap width
	const int bh = static_cast<int>(sz.height * scale + 0.5f); // Bitmap height

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
	//const FS_RECTF clip{ 0.0f, 0.0f, float(bw), float(bh) }; // To bitmap

	const int options = (FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
	FPDF_RenderPageBitmap(pFpdfBmp.get(), GetPagePtr().get(), 0, 0, bw, bh, 0, options);

	CComPtr<IWICBitmap> pWICBitmap;
	FAILED_THROW(m_pDoc->GetDirectPtr()->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));

	CComPtr<ID2D1Bitmap> pBitmap;
	FAILED_THROW(m_pDoc->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));

	SetLockBitmap(PdfBmpInfo{ pBitmap, scale });
}


/**************/
/* SML Action */
/**************/
void CPDFiumPage::None_Render(const RenderEvent& e)
{
	m_requestingScale = e.Scale;
	process_event(LoadEvent());
}
void CPDFiumPage::Loading_OnEntry() 
{
	auto fun = [this]()
	{ 
		Load();
		process_event(LoadCompletedEvent());
	};
	//fun();
	m_future = std::async(
	std::launch::async,
	async_action_wrap<decltype(fun)>,
	fun);
}

void CPDFiumPage::Loading_Render(const RenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();
	if (e.Scale != m_loadingScale) {
		process_event(ReloadEvent());
	}

	if (pbi.BitmapPtr) {
		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, e.RenderRectInWnd);
	} else {
		e.DirectPtr->DrawTextInRect(*m_pDoc->GetFormatPtr(), L"Loading Page...", e.RenderRectInWnd);
	}
}

void CPDFiumPage::Available_Render(const RenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();
	if (e.Scale != pbi.Scale) {
		process_event(ReloadEvent());
	}

	if (pbi.BitmapPtr) {
		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, e.RenderRectInWnd);
	}
}

void CPDFiumPage::WaitCancel_OnEntry()
{
	*m_spCancelThread = true;
}
void CPDFiumPage::WaitCancel_OnExit()
{
	*m_spCancelThread = false;
}

void CPDFiumPage::WaitCancel_Render(const RenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();

	if (pbi.BitmapPtr) {
		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, e.RenderRectInWnd);
	} else {
		e.DirectPtr->DrawTextInRect(*m_pDoc->GetFormatPtr(), L"Loading Page...", e.RenderRectInWnd);
	}
}

void CPDFiumPage::Error_Render(const RenderEvent& e)
{
	m_requestingScale = e.Scale;

	e.DirectPtr->DrawTextInRect(*m_pDoc->GetFormatPtr(), L"Error on Page loading.", e.RenderRectInWnd);
}


