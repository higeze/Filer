#include "PDFiumDoc.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
#include <mutex>
#include "async_catch.h"
#include "ThreadPool.h"
#include "PdfView.h"


#include "strconv.h"

namespace sml = boost::sml;

void CPDFiumDoc::Init()
{
	const FPDF_LIBRARY_CONFIG config{ 2, nullptr, nullptr, 0 };
	FPDF_InitLibraryWithConfig(&config);
}

void CPDFiumDoc::Term()
{
	FPDF_DestroyLibrary();
}

CPDFiumDoc::CPDFiumDoc(const std::shared_ptr<PdfViewProperty>& spProp, const std::wstring& path, const std::wstring& password, 
		CDirect2DWrite* pDirect, std::function<void()> changed)
	:m_pProp(spProp),
	m_path(path),
	m_password(password),
	m_pPDFium(std::make_unique<CPDFiumSingleThread>()),
	m_pDirect(pDirect),
	m_changed(changed)
{

	m_pDoc = std::move(m_pPDFium->UnqLoadDocument(wide_to_utf8(m_path).c_str(), wide_to_utf8(m_password).c_str()));

	m_pageCount = m_pPDFium->GetPageCount(m_pDoc.get());

	for (auto i = 0; i < m_pageCount; i++) {
		m_pages.emplace_back(std::make_unique<CPDFiumPage>(this, i));
	}

	for (const auto& pPage : m_pages) {
		m_sourceSize.width = (std::max)(m_sourceSize.width, pPage->GetSourceSize().width);
		m_sourceSize.height += pPage->GetSourceSize().height;
	}
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

	m_pPage = std::move(GetPDFiumPtr()->UnqLoadPage(pDoc->GetDocPtr().get(), m_index));
	m_sourceSize.width = static_cast<FLOAT>(GetPDFiumPtr()->GetPageWidth(GetPagePtr().get()));
	m_sourceSize.height = static_cast<FLOAT>(GetPDFiumPtr()->GetPageHeight(GetPagePtr().get()));
}

CPDFiumPage::~CPDFiumPage() = default;

void CPDFiumPage::Load()
{
	m_loadingScale = m_requestingScale;

	const CSizeF sz = GetSourceSize();
	const int bw = static_cast<int>(std::round(sz.width * m_requestingScale)); // Bitmap width
	const int bh = static_cast<int>(std::round(sz.height * m_requestingScale)); // Bitmap height

	BITMAPINFOHEADER bmih{};
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = bw;
	bmih.biHeight = -bh;
	bmih.biPlanes = 1; 
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	
	void* bitmapBits = nullptr;

	std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(
		::CreateDIBSection(m_pDoc->GetDirectPtr()->GetHDC(), reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0)
	);
	FALSE_THROW(pBmp);

	UNQ_FPDF_BITMAP pFpdfBmp(GetPDFiumPtr()->Bitmap_UnqCreateEx(bw, bh, FPDFBitmap_BGRx, bitmapBits, (bw * 4)));

	FALSE_THROW(pFpdfBmp);

	GetPDFiumPtr()->Bitmap_FillRect(pFpdfBmp.get(), 0, 0, bw, bh, 0xFFFFFFFF); // Fill white
	//const FS_RECTF clip{ 0.0f, 0.0f, float(bw), float(bh) }; // To bitmap

	const int options = (FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
	GetPDFiumPtr()->RenderPageBitmap(pFpdfBmp.get(), GetPagePtr().get(), 0, 0, bw, bh, 0, options);

	CComPtr<IWICBitmap> pWICBitmap;
	FAILED_THROW(m_pDoc->GetDirectPtr()->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));

	CComPtr<ID2D1Bitmap> pBitmap;
	FAILED_THROW(m_pDoc->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));

	SetLockBitmap(PdfBmpInfo{ pBitmap, m_loadingScale });
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
	
	m_future = GetPDFiumPtr()->GetThreadPtr()->enqueue(fun);

	m_findRects.clear();
	UNQ_FPDF_TEXTPAGE pTextPage(GetPDFiumPtr()->Text_UnqLoadPage(GetPagePtr().get()));
	FPDF_WIDESTRING text = reinterpret_cast<FPDF_WIDESTRING>(L"PDF");
	UNQ_FPDF_SCHHANDLE pSchHdl(GetPDFiumPtr()->Text_UnqFindStart(pTextPage.get(), text, 0, 0));
	CSizeF sz = GetSourceSize();
	while (GetPDFiumPtr()->Text_FindNext(pSchHdl.get())) {
		int index = GetPDFiumPtr()->Text_GetSchResultIndex(pSchHdl.get());
		int ch_count = GetPDFiumPtr()->Text_GetSchCount(pSchHdl.get());
		int rc_count = GetPDFiumPtr()->Text_CountRects(pTextPage.get(), index, ch_count);
		for (int i = 0; i < rc_count; i++) {
			double left, top, right, bottom;
			m_pDoc->GetPDFiumPtr()->Text_GetRect(
				pTextPage.get(), 
				i,
				&left, 
				&top, 
				&right,
				&bottom);
			m_findRects.emplace_back(
				static_cast<FLOAT>(left), 
				static_cast<FLOAT>(sz.height - top), 
				static_cast<FLOAT>(right), 
				static_cast<FLOAT>(sz.height - bottom));
		}
	}
}

void CPDFiumPage::Loading_Render(const RenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();
	if (e.Scale != m_loadingScale) {
		process_event(ReloadEvent());
	}

	if (pbi.BitmapPtr) {
		auto sz = pbi.BitmapPtr->GetSize();
		auto rc = CRectF(
				std::round(e.RenderRectInWnd.left),
				std::round(e.RenderRectInWnd.top),
				std::round(e.RenderRectInWnd.left + sz.width * e.Scale / pbi.Scale) ,
				std::round(e.RenderRectInWnd.top + sz.height * e.Scale / pbi.Scale));

		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	} else {
		e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", e.RenderRectInWnd);
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
		auto sz = pbi.BitmapPtr->GetSize();
		auto rc = CRectF(
				std::round(e.RenderRectInWnd.left),
				std::round(e.RenderRectInWnd.top),
				std::round(e.RenderRectInWnd.left + sz.width * e.Scale / pbi.Scale) ,
				std::round(e.RenderRectInWnd.top + sz.height * e.Scale / pbi.Scale));

		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

		for (const auto& ch_rc : m_findRects) {
			e.DirectPtr->FillSolidRectangle(
				*(m_pDoc->GetPropPtr()->FindHighliteFill),
				CRectF(
					rc.left + ch_rc.left * e.Scale, 
					rc.top + ch_rc.top * e.Scale,
					rc.left + ch_rc.right * e.Scale,
					rc.top + ch_rc.bottom * e.Scale));
		}

	} else {
		e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", e.RenderRectInWnd);
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
		auto sz = pbi.BitmapPtr->GetSize();
		auto rc = CRectF(
				std::round(e.RenderRectInWnd.left),
				std::round(e.RenderRectInWnd.top),
				std::round(e.RenderRectInWnd.left + sz.width * e.Scale / pbi.Scale) ,
				std::round(e.RenderRectInWnd.top + sz.height * e.Scale / pbi.Scale));

		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

	} else {
		e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", e.RenderRectInWnd);
	}
}

void CPDFiumPage::Error_Render(const RenderEvent& e)
{
	m_requestingScale = e.Scale;

	e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Error on Page loading.", e.RenderRectInWnd);
}


