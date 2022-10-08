#include "PDFPage.h"
#include "PDFDoc.h"
#include <mutex>
#include <boost/algorithm/string.hpp>
#include "async_catch.h"
#include "ThreadPool.h"
#include "PdfView.h"
#include "PDFViewport.h"
#include "MyClipboard.h"

#include "strconv.h"

namespace sml = boost::sml;

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
			*state<None> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_None_Render),
			state<None> +event<InitialLoadEvent> = state<Loading>,

			state<Loading> +on_entry<InitialLoadEvent> / call(&CPDFPage::Bitmap_InitialLoading_OnEntry),
			state<Loading> +on_entry<BitmapReloadEvent> / [](CPDFPage* p, const BitmapReloadEvent& e) { return p->Bitmap_Loading_OnEntry(e.DirectPtr, e.Scale); },
			state<Loading> +on_entry<BitmapLoadCompletedEvent> / [](CPDFPage* p, const BitmapLoadCompletedEvent& e) { return p->Bitmap_Loading_OnEntry(e.DirectPtr, e.Scale); },
			state<Loading> +on_entry<BitmapCancelCompletedEvent> / [](CPDFPage* p, const BitmapCancelCompletedEvent& e) { return p->Bitmap_Loading_OnEntry(e.DirectPtr, e.Scale); },

			state<Loading> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_Loading_Render),
			state<Loading> +event<BitmapLoadCompletedEvent> = state<Available>,
			state<Loading> +event<BitmapReloadEvent> = state<WaitCancel>,
			state<Loading> +event<BitmapErrorEvent> = state<Error>,

			state<Available> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_Available_Render),
			state<Available> +event<RenderPageSelectedTextEvent> / call(&CPDFPage::Bitmap_Available_RenderSelectedText),
			state<Available> +event<RenderPageCaretEvent> / call(&CPDFPage::Bitmap_Available_RenderCaret),
			state<Available> +event<BitmapReloadEvent> = state<Loading>,

			state<WaitCancel> +on_entry<_> / call(&CPDFPage::Bitmap_WaitCancel_OnEntry),
			state<WaitCancel> +on_exit<_> / call(&CPDFPage::Bitmap_WaitCancel_OnExit),

			state<WaitCancel> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_WaitCancel_Render),
			state<WaitCancel> +event<BitmapLoadCompletedEvent> = state<Loading>,
			state<WaitCancel> +event<BitmapCancelCompletedEvent> = state<Loading>,

			state<Error> +event<RenderPageContentEvent> / call(&CPDFPage::Bitmap_Error_Render)

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
			*state<None> +event<RenderPageFindEvent> / call(&CPDFPage::Find_None_Render),
			state<None> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_None_RenderLine),
			state<None> +event<FindLoadEvent> = state<Loading>,

			state<Loading> +on_entry<FindLoadEvent> / call(&CPDFPage::Find_Loading_OnEntry),
			state<Loading> +on_entry<FindReloadEvent> / call(&CPDFPage::Find_Loading_OnReEntry),

			state<Loading> +event<RenderPageFindEvent> / call(&CPDFPage::Find_Loading_Render),
			state<Loading> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_Loading_RenderLine),
			state<Loading> +event<FindLoadCompletedEvent> = state<Available>,
			state<Loading> +event<FindReloadEvent> = state<WaitCancel>,
			state<Loading> +event<FindErrorEvent> = state<Error>,

			state<Available> +event<RenderPageFindEvent> / call(&CPDFPage::Find_Available_Render),
			state<Available> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_Available_RenderLine),
			state<Available> +event<FindReloadEvent> = state<Loading>,

			state<WaitCancel> +on_entry<_> / call(&CPDFPage::Find_WaitCancel_OnEntry),
			state<WaitCancel> +on_exit<_> / call(&CPDFPage::Find_WaitCancel_OnExit),

			state<WaitCancel> +event<RenderPageFindEvent> / call(&CPDFPage::Find_WaitCancel_Render),
			state<WaitCancel> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_WaitCancel_RenderLine),
			state<WaitCancel> +event<FindLoadCompletedEvent> = state<Loading>,
			state<WaitCancel> +event<FindCancelCompletedEvent> = state<Loading>,

			state<Error> +event<RenderPageFindEvent> / call(&CPDFPage::Find_Error_Render),
			state<Error> +event<RenderPageFindLineEvent> / call(&CPDFPage::Find_Error_RenderLine)
			//Error handler
			//*state<Error> +exception<std::exception> = state<Error>
		);
	}
};

void CPDFPage::RenderContent(const RenderPageContentEvent& e) { process_event(e); }
void CPDFPage::RenderFind(const RenderPageFindEvent& e) { process_event(e); }
void CPDFPage::RenderFindLine(const RenderPageFindLineEvent& e) { process_event(e); }
void CPDFPage::RenderSelectedText(const RenderPageSelectedTextEvent& e) { process_event(e); }
void CPDFPage::RenderCaret(const RenderPageCaretEvent& e) { process_event(e); }



void CPDFPage::process_event(const RenderPageContentEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const RenderPageFindEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const RenderPageFindLineEvent& e) { m_pFindMachine->process_event(e); }
void CPDFPage::process_event(const RenderPageSelectedTextEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const RenderPageCaretEvent& e) { m_pBitmapMachine->process_event(e); }

void CPDFPage::process_event(const InitialLoadEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapReloadEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapLoadCompletedEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapCancelCompletedEvent& e) { m_pBitmapMachine->process_event(e); }
void CPDFPage::process_event(const BitmapErrorEvent& e) { m_pBitmapMachine->process_event(e); }

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
	m_loadingScale(0.f),
	m_pBitmapMachine(new sml::sm<BitmapMachine, boost::sml::process_queue<std::queue>>{ this }),
	m_pFindMachine(new sml::sm<FindMachine, boost::sml::process_queue<std::queue>>{ this })
{

	m_pPage = std::move(GetPDFiumPtr()->UnqLoadPage(pDoc->GetDocPtr().get(), m_index));
	m_pTextPage = std::move(GetPDFiumPtr()->Text_UnqLoadPage(GetPagePtr().get()));

	//m_sourceSize.width = static_cast<FLOAT>(GetPDFiumPtr()->GetPageWidth(GetPagePtr().get()));
	//m_sourceSize.height = static_cast<FLOAT>(GetPDFiumPtr()->GetPageHeight(GetPagePtr().get()));
	Rotate.set(GetPDFiumPtr()->Page_GetRotation(m_pPage.get()));
	Rotate.Subscribe([this](const int& value) 
		{
			GetPDFiumPtr()->Page_SetRotation(m_pPage.get(), value);
			ClearSourceSize();
		
		},(std::numeric_limits<sigslot::group_id>::min)());
}

CPDFPage::~CPDFPage() = default;

std::unique_ptr<CPDFiumSingleThread>& CPDFPage::GetPDFiumPtr() { return m_pDoc->GetPDFiumPtr(); }

//void CPDFPage::SetRotate(const int& rotate)
//{
//	if (m_rotate != rotate) {
//		GetPDFiumPtr()->Page_SetRotation(m_pPage.get(), rotate);
//		m_rotate = GetPDFiumPtr()->Page_GetRotation(m_pPage.get());
//		PropertyChanged("Rotate");
//	}
//	
//}

void CPDFPage::LoadSourceSize()
{
	m_optSourceSize = CSizeF();
	m_optSourceSize->width = static_cast<FLOAT>(GetPDFiumPtr()->GetPageWidth(GetPagePtr().get()));
	m_optSourceSize->height = static_cast<FLOAT>(GetPDFiumPtr()->GetPageHeight(GetPagePtr().get()));
}

void CPDFPage::LoadBitmap(CDirect2DWrite* pDirect, const FLOAT& scale)
{
	m_loadingScale = scale;
	m_loadingRotate = Rotate.get();

	const CSizeF sz = GetSourceSize();
	const int bw = static_cast<int>(std::round(sz.width * scale)); // Bitmap width
	const int bh = static_cast<int>(std::round(sz.height * scale)); // Bitmap height

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
		::CreateDIBSection(pDirect->GetHDC(), reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0)
	);
	FALSE_THROW(pBmp);

	UNQ_FPDF_BITMAP pFpdfBmp(GetPDFiumPtr()->Bitmap_UnqCreateEx(bw, bh, FPDFBitmap_BGRx, bitmapBits, (bw * 4)));

	FALSE_THROW(pFpdfBmp);

	GetPDFiumPtr()->Bitmap_FillRect(pFpdfBmp.get(), 0, 0, bw, bh, 0xFFFFFFFF); // Fill white
	//const FS_RECTF clip{ 0.0f, 0.0f, float(bw), float(bh) }; // To bitmap

	const int options = (FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);
	GetPDFiumPtr()->RenderPageBitmap(pFpdfBmp.get(), GetPagePtr().get(), 0, 0, bw, bh, 0, options);

	CComPtr<IWICBitmap> pWICBitmap;
	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBitmap));

	CComPtr<ID2D1Bitmap> pBitmap;
	FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap));
	auto mysize = pBitmap->GetSize().width * pBitmap->GetSize().height * 4;
	m_pDoc->totalsize += mysize;
	::OutputDebugString(std::format(L"{}:\t{}\t{}\r\n",this->m_index, mysize, m_pDoc->totalsize).c_str());
	SetLockBitmap(PdfBmpInfo{ pBitmap, m_loadingScale, m_loadingRotate });
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
	SetLockFind(PdfFndInfo(rects, find.c_str()));
}

void CPDFPage::LoadText()
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

		CSizeF sz = GetSourceSize();
		//FLOAT prevBottom = sz.height;
		//FLOAT curBottom = sz.height;
		//FLOAT curTop = 0.f;
		//std::size_t curbegIndex = 0;
		//std::size_t curlastIndex = 0;
		//std::size_t prevbegIndex = 0;
		//std::size_t prevlastIndex = 0;

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
				mouseRects[i].right = (std::min)(cursorRects[i - 1].right + cursorRects[i - 1].Width() * 0.5f, sz.width);
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
					mouseRects[i].right = (std::min)(cursorRects[i].right - cursorRects[i].Width() * 0.5f, sz.width);
				} else {
					mouseRects[i].right = (std::min)(cursorRects[i].right + cursorRects[i].Width() * 0.5f, (cursorRects[i].right + cursorRects[i + 1].left) * 0.5f);
				}
			}
			//top & bottom
			//Since line order is not always top to bottom, it's hard to adjust context.
			mouseRects[i].top = (std::min)(cursorRects[i].top + (-cursorRects[i].Height()) * 0.2f, sz.height);
			mouseRects[i].bottom = (std::max)(cursorRects[i].bottom - (-cursorRects[i].Height()) * 0.2f, 0.f);
		}

		SetLockTxt(PdfTxtInfo(orgCharRects, cursorRects, mouseRects, text));
	} else {
		SetLockTxt(PdfTxtInfo(std::vector<CRectF>(), std::vector<CRectF>(), std::vector<CRectF>(), text));
	}
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
	return GetLockTxt().CRLFRects[index];
}

int CPDFPage::GetTextSize()
{
	auto txt = GetLockTxt();
	return txt.String.size();
}
std::wstring CPDFPage::GetText()
{
	auto txt = GetLockTxt();
	return txt.String;	
}



/**************/
/* SML Action */
/**************/
void CPDFPage::Bitmap_None_Render(const RenderPageContentEvent& e)
{
	process_event(InitialLoadEvent(e.DirectPtr, e.Scale));
}
void CPDFPage::Bitmap_InitialLoading_OnEntry(const InitialLoadEvent& e) 
{
	auto fun = [this, e]()
	{ 
		LoadBitmap(e.DirectPtr, e.Scale);
		LoadText();
		process_event(BitmapLoadCompletedEvent(e.DirectPtr, e.Scale));
	};
	
	m_futureBitmap = GetPDFiumPtr()->GetThreadPtr()->enqueue(fun);
}
void CPDFPage::Bitmap_Loading_OnEntry(CDirect2DWrite* pDirect, const FLOAT& scale) 
{
	auto fun = [this, pDirect, scale]()
	{ 
		LoadBitmap(pDirect, scale);
		process_event(BitmapLoadCompletedEvent(pDirect, scale));
	};
	
	m_futureBitmap = GetPDFiumPtr()->GetThreadPtr()->enqueue(fun);
}

void CPDFPage::Bitmap_Loading_Render(const RenderPageContentEvent& e) 
{
	auto pbi = GetLockBitmap();
	if (e.Scale != m_loadingScale || Rotate.get() != m_loadingRotate) {
		process_event(BitmapReloadEvent(e.DirectPtr, e.Scale));
	}
	if (pbi.BitmapPtr) {
		auto sz = pbi.BitmapPtr->GetSize();
		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
		auto rc = CRectF(
				std::round(ptInWnd.x),
				std::round(ptInWnd.y),
				std::round(ptInWnd.x + sz.width * e.Scale / pbi.Scale),
				std::round(ptInWnd.y + sz.height * e.Scale / pbi.Scale));

		if (pbi.BitmapPtr) {
			e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		} else {
			e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", rc);
		}
	}
}

void CPDFPage::Bitmap_Available_Render(const RenderPageContentEvent& e) 
{
	auto pbi = GetLockBitmap();
	if (e.Scale != pbi.Scale || Rotate.get() != pbi.Rotate) {
		process_event(BitmapReloadEvent(e.DirectPtr, e.Scale));
	}
	if (pbi.BitmapPtr) {
		auto sz = pbi.BitmapPtr->GetSize();
		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
		auto rc = CRectF(
				std::round(ptInWnd.x),
				std::round(ptInWnd.y),
				std::round(ptInWnd.x + sz.width * e.Scale / pbi.Scale),
				std::round(ptInWnd.y + sz.height * e.Scale / pbi.Scale));

		if (pbi.BitmapPtr) {
			e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			if (e.Debug) {
				auto txt = GetLockTxt();
				for (const auto& rc : txt.MouseRects) {
					e.DirectPtr->DrawSolidRectangle(SolidLine(1.f, 0.f, 0.f, 1.f, 1.f), e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, rc));
				}
			}
		} else {
			e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", rc);
		}
	}
}

void CPDFPage::Bitmap_Available_RenderSelectedText(const RenderPageSelectedTextEvent& e)
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
		auto rcInPdfiumPage = CRectF(
			static_cast<FLOAT>(left),
			static_cast<FLOAT>(top),
			static_cast<FLOAT>(right),
			static_cast<FLOAT>(bottom));
		auto rcSelectInWnd = e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, rcInPdfiumPage);
		e.DirectPtr->FillSolidRectangle(
				*(m_pDoc->GetPropPtr()->SelectedFill), rcSelectInWnd);
	}
}

void CPDFPage::Bitmap_Available_RenderCaret(const RenderPageCaretEvent& e)
{
	auto txt = GetLockTxt();
	if (!txt.CRLFRects.empty()) {
		auto rcInPdfiumPage = txt.CRLFRects[e.CharIndex];
		rcInPdfiumPage.right = rcInPdfiumPage.left + 1.f;

		e.DirectPtr->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		e.DirectPtr->FillSolidRectangle(m_pDoc->GetPropPtr()->Format->Color, e.ViewportPtr->PdfiumPageToWnd(e.PageIndex, rcInPdfiumPage));
		e.DirectPtr->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
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

void CPDFPage::Bitmap_WaitCancel_Render(const RenderPageContentEvent& e) 
{
	auto pbi = GetLockBitmap();
	if (pbi.BitmapPtr) {
		auto sz = pbi.BitmapPtr->GetSize();
		auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
		auto rc = CRectF(
				std::round(ptInWnd.x),
				std::round(ptInWnd.y),
				std::round(ptInWnd.x + sz.width * e.Scale / pbi.Scale),
				std::round(ptInWnd.y + sz.height * e.Scale / pbi.Scale));

		if (pbi.BitmapPtr) {
			e.DirectPtr->GetD2DDeviceContext()->DrawBitmap(pbi.BitmapPtr, rc, 1.f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		} else {
			e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Loading Page...", rc);
		}
	}
}

void CPDFPage::Bitmap_Error_Render(const RenderPageContentEvent& e)
{
	auto pbi = GetLockBitmap();
	auto sz = pbi.BitmapPtr->GetSize();
	auto ptInWnd = e.ViewportPtr->PageToWnd(e.PageIndex, CPointF());
	auto rc = CRectF(
			std::round(ptInWnd.x),
			std::round(ptInWnd.y),
			std::round(ptInWnd.x + sz.width * e.Scale / pbi.Scale) ,
			std::round(ptInWnd.y + sz.height * e.Scale / pbi.Scale));

	e.DirectPtr->DrawTextInRect(*(m_pDoc->GetPropPtr()->Format), L"Error on Page loading.", rc);
}

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
	auto fnd = GetLockFind();
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
	auto fnd = GetLockFind();
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

void CPDFPage::Find_WaitCancel_OnEntry()
{
	*m_spCancelFindThread = true;
}
void CPDFPage::Find_WaitCancel_OnExit()
{
	*m_spCancelFindThread = false;
}