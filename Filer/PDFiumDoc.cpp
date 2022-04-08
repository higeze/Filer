#include "PDFiumDoc.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
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

CPDFiumDoc::CPDFiumDoc(CDirect2DWrite* pDirect, const std::shared_ptr<FormatF>& pFormat)
	:m_pDirect(pDirect),
	m_pFormat(pFormat){}

CPDFiumDoc::~CPDFiumDoc() = default;

void CPDFiumDoc::Open(const std::wstring& path, const std::wstring& password, std::function<void()> changed)
{
	m_pDoc.reset(FPDF_LoadDocument(wide_to_utf8(path).c_str(), wide_to_utf8(password).c_str()));
	m_count = FPDF_GetPageCount(m_pDoc.get());
	for (auto i = 0; i < m_count; i++) {
		m_pages.push_back(std::make_unique<CPDFiumPage>(this, i));
	}
}

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
	//GetSourceSize = [sz = CSizeF(), this]()mutable->CSizeF&
	//{		
	//	if (sz.width == 0 || sz.height == 0) {
	//		CComPtr<abipdf::IPdfPage> pPage;
	//		m_pPdf->GetDocument().first->GetPage(m_pageIndex, &pPage);
	//		ABI::Windows::Foundation::Size size;
	//		pPage->get_Size(&size);
	//		sz.width = size.Width;
	//		sz.height = size.Height;
	//	}
	//	return sz;
	//};
}

CPDFiumPage::~CPDFiumPage()
{
	*m_spCancelThread = true;
}

void CPDFiumPage::Clear()
{
	//GetSourceSize() = CSizeF();
	SetLockBitmap({ CComPtr<ID2D1Bitmap>(), 0.f });
}

void CPDFiumPage::Load()
{
	m_pPage.reset(FPDF_LoadPage(m_pDoc->GetDocPtr().get(), m_index));
	auto scale = m_requestingScale;
	m_loadingScale = scale;

	auto width = static_cast<int>(FPDF_GetPageWidth(m_pPage.get()) * scale + 0.5f);
	auto height = static_cast<int>(FPDF_GetPageHeight(m_pPage.get()) * scale + 0.5f);


	const int bw = static_cast<int>(FPDF_GetPageWidth(m_pPage.get()) * scale + 0.5f); // Bitmap width
	const int bh = static_cast<int>(FPDF_GetPageHeight(m_pPage.get()) * scale + 0.5f); // Bitmap height

	BITMAPINFOHEADER bmih; RtlSecureZeroMemory(&bmih, sizeof(bmih));
	bmih.biSize = sizeof(bmih); bmih.biWidth = bw; bmih.biHeight = -bh;
	bmih.biPlanes = 1; bmih.biBitCount = 32; bmih.biCompression = BI_RGB;
	bmih.biSizeImage = (bw * bh * 4); void* bitmapBits = nullptr;
	const BITMAPINFO* bmi = (BITMAPINFO*)&bmih;

	HBITMAP hBmp = nullptr;
	if (hBmp = CreateDIBSection(m_pDoc->GetDirectPtr()->GetHDC(), bmi, DIB_RGB_COLORS, &bitmapBits, nullptr, 0)) {
		if (FPDF_BITMAP pdfBitmap = FPDFBitmap_CreateEx(bw, bh, FPDFBitmap_BGRx, bitmapBits, (bw * 4))) {
			FPDFBitmap_FillRect(pdfBitmap, 0, 0, bw, bh, 0xFFFFFFFF); // Fill white

			//const FS_MATRIX matrix = {float(scale), 0.0f, 0.0f, float(-scale), 0.0f, float(bh)};
			const FS_MATRIX matrix{ float(scale), 0.0f, 0.0f, float(scale), 0.0f, 0.0f };

			const FS_RECTF clip{ 0.0f, 0.0f, float(bw), float(bh) }; // To bitmap

			const int options = (FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);

			//FPDF_RenderPageBitmapWithMatrix(pdfBitmap, m_pdfPage, &matrix, &clip, options);
			FPDF_RenderPageBitmap(pdfBitmap, m_pPage.get(), clip.left, clip.top, clip.right, clip.bottom, 0, options);

			FPDFBitmap_Destroy(pdfBitmap); pdfBitmap = nullptr;
		}
	}

	CComPtr<IWICBitmap> pWICBitmap;
	FAILED_THROW(m_pDoc->GetDirectPtr()->GetWICImagingFactory()->CreateBitmapFromHBITMAP(hBmp, nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));

	CComPtr<IWICFormatConverter> pWICFormatConverter;
	FAILED_THROW(m_pDoc->GetDirectPtr()->GetWICImagingFactory()->CreateFormatConverter(&pWICFormatConverter));

	FAILED_THROW(pWICFormatConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut));

	double dpix = 96.0f, dpiy = 96.0f;
	FAILED_THROW(pWICFormatConverter->GetResolution(&dpix, &dpiy));

	D2D1_BITMAP_PROPERTIES bitmapProps;
	//bitmapProps.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
	bitmapProps.dpiX = (FLOAT)dpix;
	bitmapProps.dpiY = (FLOAT)dpiy;
	bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	//bitmapProps.colorContext = nullptr;

	CComPtr<ID2D1Bitmap> pBitmap;
	FAILED_THROW(m_pDoc->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, bitmapProps, &pBitmap));
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
	auto fun = [this] { Load(); };
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


