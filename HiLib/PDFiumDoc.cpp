#include "PDFiumDoc.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
#include <mutex>
#include <boost/algorithm/string.hpp>
#include "async_catch.h"
#include "ThreadPool.h"
#include "PdfView.h"
#include "PDFViewport.h"


#include "strconv.h"

namespace sml = boost::sml;

void CPDFDoc::Init()
{
	const FPDF_LIBRARY_CONFIG config{ 2, nullptr, nullptr, 0 };
	FPDF_InitLibraryWithConfig(&config);
}

void CPDFDoc::Term()
{
	FPDF_DestroyLibrary();
}

CPDFDoc::CPDFDoc(const std::shared_ptr<PdfViewProperty>& spProp, const std::wstring& path, const std::wstring& password, 
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
		m_pages.emplace_back(std::make_unique<CPDFPage>(this, i));
	}

	for (const auto& pPage : m_pages) {
		m_sourceRectsInDoc.emplace_back(0.f, m_sourceSize.height, pPage->GetSourceSize().width, m_sourceSize.height + pPage->GetSourceSize().height);

		m_sourceSize.width = (std::max)(m_sourceSize.width, pPage->GetSourceSize().width);
		m_sourceSize.height += pPage->GetSourceSize().height;
	}
}

CPDFDoc::~CPDFDoc() = default;

void CPDFDoc::RenderHighliteLine(const FindRenderLineEvent& e)
{
	FLOAT fullHeight = GetSourceSize().height;
	FLOAT top = 0.f;
	for (auto i = 0; i < GetPageCount(); i++) {
		auto rectHighlite = CRectF(
			e.RenderRectInWnd.left + 2.f,
			e.RenderRectInWnd.top + e.RenderRectInWnd.Height() * m_sourceRectsInDoc[i].top / fullHeight,
			e.RenderRectInWnd.right - 2.f,
			e.RenderRectInWnd.top + e.RenderRectInWnd.Height() * m_sourceRectsInDoc[i].bottom / fullHeight);

		GetPage(i)->RenderHighliteLine(FindRenderLineEvent(e.DirectPtr, e.ViewportPtr, rectHighlite, e.Scale, e.Find));
	}
}

/************/
/* CPdfPage */
/************/

struct CPDFPage::MachineBase
{
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
};


struct CPDFPage::BitmapMachine: public CPDFPage::MachineBase
{
	//Status
	class None {};
	class Loading {};
	class Available {};
	class WaitCancel {};
	class Error {};

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<None> +event<BitmapRenderEvent> / call(&CPDFPage::Bitmap_None_Render),
			state<None> +event<BitmapLoadEvent> = state<Loading>,

			state<Loading> +on_entry<_> / call(&CPDFPage::Bitmap_Loading_OnEntry),

			state<Loading> +event<BitmapRenderEvent> / call(&CPDFPage::Bitmap_Loading_Render),
			state<Loading> +event<BitmapLoadCompletedEvent> = state<Available>,
			state<Loading> +event<BitmapReloadEvent> = state<WaitCancel>,
			state<Loading> +event<BitmapErrorEvent> = state<Error>,

			state<Available> +event<BitmapRenderEvent> / call(&CPDFPage::Bitmap_Available_Render),
			state<Available> +event<BitmapReloadEvent> = state<Loading>,

			state<WaitCancel> +on_entry<_> / call(&CPDFPage::Bitmap_WaitCancel_OnEntry),
			state<WaitCancel> +on_exit<_> / call(&CPDFPage::Bitmap_WaitCancel_OnExit),

			state<WaitCancel> +event<BitmapRenderEvent> / call(&CPDFPage::Bitmap_WaitCancel_Render),
			state<WaitCancel> +event<BitmapLoadCompletedEvent> = state<Loading>,
			state<WaitCancel> +event<BitmapCancelCompletedEvent> = state<Loading>,

			state<Error> +event<BitmapRenderEvent> / call(&CPDFPage::Bitmap_Error_Render)

			//Error handler
			//*state<Error> +exception<std::exception> = state<Error>
		);
	}
};

struct CPDFPage::FindMachine: public CPDFPage::MachineBase
{
	//Status
	class None {};
	class Loading {};
	class Available {};
	class WaitCancel {};
	class Error {};

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<None> +event<FindRenderEvent> / call(&CPDFPage::Find_None_Render),
			state<None> +event<FindRenderLineEvent> / call(&CPDFPage::Find_None_RenderLine),
			state<None> +event<FindLoadEvent> = state<Loading>,

			state<Loading> +on_entry<FindLoadEvent> / call(&CPDFPage::Find_Loading_OnEntry),
			state<Loading> +on_entry<FindReloadEvent> / call(&CPDFPage::Find_Loading_OnReEntry),

			state<Loading> +event<FindRenderEvent> / call(&CPDFPage::Find_Loading_Render),
			state<Loading> +event<FindRenderLineEvent> / call(&CPDFPage::Find_Loading_RenderLine),
			state<Loading> +event<FindLoadCompletedEvent> = state<Available>,
			state<Loading> +event<FindReloadEvent> = state<WaitCancel>,
			state<Loading> +event<FindErrorEvent> = state<Error>,

			state<Available> +event<FindRenderEvent> / call(&CPDFPage::Find_Available_Render),
			state<Available> +event<FindRenderLineEvent> / call(&CPDFPage::Find_Available_RenderLine),
			state<Available> +event<FindReloadEvent> = state<Loading>,

			state<WaitCancel> +on_entry<_> / call(&CPDFPage::Find_WaitCancel_OnEntry),
			state<WaitCancel> +on_exit<_> / call(&CPDFPage::Find_WaitCancel_OnExit),

			state<WaitCancel> +event<FindRenderEvent> / call(&CPDFPage::Find_WaitCancel_Render),
			state<WaitCancel> +event<FindRenderLineEvent> / call(&CPDFPage::Find_WaitCancel_RenderLine),
			state<WaitCancel> +event<FindLoadCompletedEvent> = state<Loading>,
			state<WaitCancel> +event<FindCancelCompletedEvent> = state<Loading>,

			state<Error> +event<FindRenderEvent> / call(&CPDFPage::Find_Error_Render),
			state<Error> +event<FindRenderLineEvent> / call(&CPDFPage::Find_Error_RenderLine)
			//Error handler
			//*state<Error> +exception<std::exception> = state<Error>
		);
	}
};


void CPDFPage::process_event(const BitmapRenderEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapLoadEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapReloadEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapLoadCompletedEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapCancelCompletedEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapErrorEvent& e) { m_pBitmapMachine->process_event(e); }

void CPDFPage::process_event(const FindRenderEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindRenderLineEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindLoadEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindReloadEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindLoadCompletedEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindCancelCompletedEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const FindErrorEvent& e) { m_pFindMachine->process_event(e); }

CPDFPage::CPDFPage(CPDFDoc* pDoc, int index )
	:m_pDoc(pDoc), m_index(index),
	m_spCancelBitmapThread(std::make_shared<bool>(false)),
	m_spCancelFindThread(std::make_shared<bool>(false)),
	m_bmp{ CComPtr<ID2D1Bitmap>(), 0.f },
	m_fnd{ std::vector<CRectF>(), L"" },
	m_loadingScale(0.f), m_requestingScale(0.f),
	m_pBitmapMachine(new sml::sm<BitmapMachine, boost::sml::process_queue<std::queue>>{ this }),
	m_pFindMachine(new sml::sm<FindMachine, boost::sml::process_queue<std::queue>>{ this })
{

	m_pPage = std::move(GetPDFiumPtr()->UnqLoadPage(pDoc->GetDocPtr().get(), m_index));
	m_sourceSize.width = static_cast<FLOAT>(GetPDFiumPtr()->GetPageWidth(GetPagePtr().get()));
	m_sourceSize.height = static_cast<FLOAT>(GetPDFiumPtr()->GetPageHeight(GetPagePtr().get()));
}

CPDFPage::~CPDFPage() = default;

void CPDFPage::LoadBitmap()
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

void CPDFPage::LoadFind(const std::wstring& find_string)
{
	auto find = boost::trim_copy(find_string);
	auto rects = std::vector<CRectF>();
	if(find.empty())
	{
		SetLockFind(PdfFndInfo(rects, find.c_str()));
		return;
	}

	UNQ_FPDF_TEXTPAGE pTextPage(GetPDFiumPtr()->Text_UnqLoadPage(GetPagePtr().get()));
	FPDF_WIDESTRING text = reinterpret_cast<FPDF_WIDESTRING>(find.c_str());
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
			rects.emplace_back(
				static_cast<FLOAT>(left), 
				static_cast<FLOAT>(sz.height - top), 
				static_cast<FLOAT>(right), 
				static_cast<FLOAT>(sz.height - bottom));
		}
	}
	SetLockFind(PdfFndInfo(rects, find.c_str()));
}

void CPDFPage::LoadText()
{
	//TextPage
	UNQ_FPDF_TEXTPAGE pTextPage(GetPDFiumPtr()->Text_UnqLoadPage(GetPagePtr().get()));
	int char_count = GetPDFiumPtr()->Text_CountChars(pTextPage.get());
	//Str
	std::wstring text;
	int text_count = GetPDFiumPtr()->Text_GetText(pTextPage.get(), 0, char_count, reinterpret_cast<unsigned short*>(::GetBuffer(text, 10000)));
	::ReleaseBuffer(text);
	//Original Char Rects
	auto orgCharRects = std::vector<CRectF>();
	for (auto i = 0; i < char_count; i++) {
		int rect_count = GetPDFiumPtr()->Text_CountRects(pTextPage.get(), i, 1);
		if (rect_count == 1) {
			double left, top, right, bottom = 0.f;
			m_pDoc->GetPDFiumPtr()->Text_GetRect(
				pTextPage.get(),
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
	//Char Rects modified for CRLF
	auto crlfCharRects = orgCharRects;
	for (std::size_t i = 0; i < crlfCharRects.size(); i++) {
		//TODO if i == 0 & isCRLF
		bool isCRLF =  text[i] == L'\r' || text[i] == L'\n';
		if (isCRLF) {
			crlfCharRects[i].SetRect(
				crlfCharRects[i - 1].right,
				crlfCharRects[i - 1].top,
				crlfCharRects[i - 1].right,
				crlfCharRects[i - 1].bottom
			);
		}
	}
	//Mouse Rects
	auto mouseRects = crlfCharRects;
	auto sz = GetSourceSize();
	auto prevBottom = sz.height;
	auto curBottom = sz.height;
	for (std::size_t i = 0; i < crlfCharRects.size(); i++) {
		bool isFirstCharInLine = i == 0 || crlfCharRects[i].CenterY() < crlfCharRects[i-1].bottom;
		bool isLastCharInLine = i == (text.size() - 1) || crlfCharRects[i].bottom > crlfCharRects[i+1].CenterY();
		bool isLastLine = crlfCharRects[i].bottom < crlfCharRects.back().CenterY();

		if (isFirstCharInLine) {
			prevBottom = curBottom;
		}
		curBottom = std::min(curBottom, crlfCharRects[i].bottom);

		//top
		mouseRects[i].top = prevBottom;
		//left
		if (isFirstCharInLine) {
			mouseRects[i].left = 0;
		}
		//right
		if (isLastCharInLine) {
			mouseRects[i].right = sz.width;
		}
		//bottom
		if (isLastLine) {
			mouseRects[i].bottom = 0;
		}
	}

	SetLockTxt(PdfTxtInfo(orgCharRects, mouseRects, text));
}

int CPDFPage::GetCursorCharIndexAtPos(const CPointF& ptInPdfiumPage)
{
	const auto& txt = GetLockTxt();
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
	return GetLockTxt().Rects[index];
}


/**************/
/* SML Action */
/**************/
void CPDFPage::Bitmap_None_Render(const BitmapRenderEvent& e)
{
	m_requestingScale = e.Scale;
	process_event(BitmapLoadEvent());
}
void CPDFPage::Bitmap_Loading_OnEntry() 
{
	auto fun = [this]()
	{ 
		LoadBitmap();
		LoadText();
		process_event(BitmapLoadCompletedEvent());
	};
	
	m_futureBitmap = GetPDFiumPtr()->GetThreadPtr()->enqueue(fun);
}

void CPDFPage::Bitmap_Loading_Render(const BitmapRenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();
	if (e.Scale != m_loadingScale) {
		process_event(BitmapReloadEvent());
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

void CPDFPage::Bitmap_Available_Render(const BitmapRenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();
	if (e.Scale != pbi.Scale) {
		process_event(BitmapReloadEvent());
	}

	if (pbi.BitmapPtr) {
		auto sz = pbi.BitmapPtr->GetSize();
		auto rc = CRectF(
				std::round(e.RenderRectInWnd.left),
				std::round(e.RenderRectInWnd.top),
				std::round(e.RenderRectInWnd.left + sz.width * e.Scale / pbi.Scale) ,
				std::round(e.RenderRectInWnd.top + sz.height * e.Scale / pbi.Scale));

		e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

		auto txt = GetLockTxt();
		for (const auto& rc : txt.MouseRects) {
			e.DirectPtr->DrawSolidRectangle(SolidLine(1.f, 0.f, 0.f, 1.f, 1.f), e.ViewportPtr->PdfiumPageToWnd(0, rc, e.Scale));
		}


	} else {
		e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", e.RenderRectInWnd);
	}
}


void CPDFPage::Bitmap_WaitCancel_OnEntry()
{
	*m_spCancelBitmapThread = true;
}
void CPDFPage::Bitmap_WaitCancel_OnExit()
{
	*m_spCancelBitmapThread = false;
}

void CPDFPage::Bitmap_WaitCancel_Render(const BitmapRenderEvent& e) 
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

void CPDFPage::Bitmap_Error_Render(const BitmapRenderEvent& e)
{
	m_requestingScale = e.Scale;

	e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Error on Page loading.", e.RenderRectInWnd);
}

/********/
/* Find */
/********/

void CPDFPage::Find_None_Render(const FindRenderEvent& e)
{
	process_event(FindLoadEvent{e.Find});
}

void CPDFPage::Find_None_RenderLine(const FindRenderLineEvent& e)
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

void CPDFPage::Find_Loading_Render(const FindRenderEvent& e) {}
void CPDFPage::Find_Loading_RenderLine(const FindRenderLineEvent& e) {}

void CPDFPage::Find_Available_Render(const FindRenderEvent& e) 
{
	auto fnd = GetLockFind();
	if (e.Find != fnd.Find) {
		process_event(FindReloadEvent{ e.Find });
	}

	for (const auto& ch_rc : fnd.FindRects) {
		e.DirectPtr->FillSolidRectangle(
			*(m_pDoc->GetPropPtr()->FindHighliteFill),
			CRectF(
				e.RenderRectInWnd.left + ch_rc.left * e.Scale, 
				e.RenderRectInWnd.top + ch_rc.top * e.Scale,
				e.RenderRectInWnd.left + ch_rc.right * e.Scale,
				e.RenderRectInWnd.top + ch_rc.bottom * e.Scale));
	}

	//auto txt = GetLockTxt();
	//
	//for (const auto& rc : txt.Rects) {
	//	e.DirectPtr->FillSolidRectangle(
	//		*(m_pDoc->GetPropPtr()->FindHighliteFill),
	//		CRectF(
	//			e.RenderRectInWnd.left + rc.left * e.Scale, 
	//			e.RenderRectInWnd.top + rc.top * e.Scale,
	//			e.RenderRectInWnd.left + rc.right * e.Scale,
	//			e.RenderRectInWnd.top + rc.bottom * e.Scale));
	//}
}

void CPDFPage::Find_Available_RenderLine(const FindRenderLineEvent& e) 
{
	auto fnd = GetLockFind();
	if (e.Find != fnd.Find) {
		process_event(FindReloadEvent{ e.Find });
	}

	CSizeF srcSize = GetSourceSize();
	SolidFill fill(*m_pDoc->GetPropPtr()->FindHighliteFill);
	fill.Color.a = 1.f;

	for (const auto& ch_rc : fnd.FindRects) {
		auto rectHighlite = CRectF(
				e.RenderRectInWnd.left,
				e.RenderRectInWnd.top + e.RenderRectInWnd.Height() * ch_rc.top / srcSize.height,
				e.RenderRectInWnd.right,
				e.RenderRectInWnd.top + e.RenderRectInWnd.Height() * ch_rc.bottom / srcSize.height);
		if (rectHighlite.Height() < 1.f) {
			rectHighlite.bottom = rectHighlite.top + 1.f;
		}
		e.DirectPtr->FillSolidRectangle(fill, rectHighlite);
	}
}

void CPDFPage::Find_WaitCancel_OnEntry()
{
	*m_spCancelFindThread = true;
}
void CPDFPage::Find_WaitCancel_OnExit()
{
	*m_spCancelFindThread = false;
}

void CPDFPage::Find_WaitCancel_Render(const FindRenderEvent& e) {}
void CPDFPage::Find_WaitCancel_RenderLine(const FindRenderLineEvent& e) {}
void CPDFPage::Find_Error_Render(const FindRenderEvent& e){}
void CPDFPage::Find_Error_RenderLine(const FindRenderLineEvent& e){}