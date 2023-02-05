#include "PDFPage.h"
#include "PDFDoc.h"
#include <mutex>
#include <boost/algorithm/string.hpp>
#include "async_catch.h"
#include "ThreadPool.h"
#include "MyUniqueHandle.h"
#include "MyClipboard.h"

#include "strconv.h"
#include <math.h>
#include "ThreadPool.h"

/************/
/* CPdfPage */
/************/
//struct CPDFPage::MachineBase
//{
//	template<class TRect, class R, class E>
//	auto call(R(TRect::* f)(E))const
//	{
//		return [f](TRect* self, E e, boost::sml::back::process<E> process) { return (self->*f)(e); };
//	}
//	
//	template<class TRect, class R>
//	auto call(R(TRect::* f)())const
//	{
//		return [f](TRect* self) { return (self->*f)(); };
//	}
//};
//struct CPDFPage::BitmapMachine: public CPDFPage::MachineBase
//{
//	//Status
//	class None {};
//	class Loading {};
//	class Available {};
//	class Error {};
//
//	auto operator()() const noexcept
//	{
//		using namespace sml;
//		return make_transition_table(
//			*state<None> +event<GetBitmapEvent> = state<Loading>,
//
//			state<Loading> +on_entry<GetBitmapEvent> / call(&CPDFPage::BitmapLoading_OnEntry),
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
CPDFPage::CPDFPage(CPDFDoc* pDoc, int index )
	:m_pDoc(pDoc), m_index(index)
{
	Rotate.set(GetPDFiumPtr()->Page_GetRotation(m_index));
	Rotate.Subscribe([this](const int& value) 
		{
			GetPDFiumPtr()->Page_SetRotation(m_index, value);
			//m_optBitmap.reset();
			m_optSize.reset();
			m_optTextOrgRects.reset();
			m_optTextOrgCursorRects.reset();
			m_optTextCursorRects.reset();
			m_optTextOrgMouseRects.reset();
			m_optTextMouseRects.reset();
			m_optFind.reset();
			m_optSelectedText.reset();

		},(std::numeric_limits<sigslot::group_id>::min)());
}

CPDFPage::~CPDFPage() = default;

std::unique_ptr<CPDFiumMultiThread>& CPDFPage::GetPDFiumPtr() { return m_pDoc->GetPDFiumPtr(); }
const std::unique_ptr<CPDFiumMultiThread>& CPDFPage::GetPDFiumPtr() const { return m_pDoc->GetPDFiumPtr(); }

const CSizeF& CPDFPage::GetSize() const
{
	if (!m_optSize.has_value()) {
		m_optSize.emplace(GetPDFiumPtr()->GetPageWidthF(m_index),
			GetPDFiumPtr()->GetPageHeightF(m_index));
	}
	return m_optSize.value();
}

const std::wstring& CPDFPage::GetText() const
{
	if (!m_optText.has_value()) {
		int charCount = GetPDFiumPtr()->Text_CountChars(m_index);
		std::wstring text(charCount, 0);
		int textCount = GetPDFiumPtr()->Text_GetText(m_index, 0, charCount, reinterpret_cast<unsigned short*>(text.data()));
		m_optText.emplace(text);
	}
	return m_optText.value();
}

int CPDFPage::GetTextSize() const
{
	return GetText().size();
}

const std::vector<CRectF>& CPDFPage::GetTextOrgRects() const
{
	if (!m_optTextOrgRects.has_value()) {
		std::vector<CRectF> textRects;
		int charCount = GetPDFiumPtr()->Text_CountChars(m_index);
		for (auto i = 0; i < charCount; i++) {
			int rect_count = GetPDFiumPtr()->Text_CountRects(m_index, i, 1);
			if (rect_count == 1) {
				double left, top, right, bottom = 0.f;
				m_pDoc->GetPDFiumPtr()->Text_GetRect(
					m_index,
					0,
					&left,
					&top,
					&right,
					&bottom);
				textRects.emplace_back(
					static_cast<FLOAT>(left),
					static_cast<FLOAT>(top),
					static_cast<FLOAT>(right),
					static_cast<FLOAT>(bottom));
			}
		}
		m_optTextOrgRects.emplace(textRects);
	}
	return m_optTextOrgRects.value();
}
auto isCRorLF = [](const wchar_t ch)->bool { return ch == L'\r' || ch == L'\n'; };
auto is_space = [](const wchar_t ch)->bool { return ch == L' ' || ch == L'@'; };
auto is_cr = [](const wchar_t ch)->bool { return ch == L'\r'; };
auto is_lf = [](const wchar_t ch)->bool { return ch == L'\n'; };

const std::vector<CRectF>& CPDFPage::GetTextOrgCursorRects() const
{
	if (!m_optTextOrgCursorRects.has_value()){
		auto cursorRects = GetTextOrgRects();
		if (!cursorRects.empty()) {
			for (std::size_t i = 1; i < cursorRects.size(); i++) {//If i = 0  is Space or CRLF, ignore
				bool isFirst = i == 0;
				bool isAfterLF = !isFirst && is_lf(GetText()[i - 1]);
				if (is_space(GetText()[i])) {
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
				if (isCRorLF(GetText()[i])) {
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
		}
		m_optTextOrgCursorRects.emplace(cursorRects);
	}
	return m_optTextOrgCursorRects.value();
}

const std::vector<CRectF>& CPDFPage::GetTextCursorRects() const
{
	if (!m_optTextCursorRects.has_value()) {
		m_optTextCursorRects = GetTextOrgCursorRects();
		RotateRects(m_optTextCursorRects.value(), Rotate.get());
	}
	return m_optTextCursorRects.value();
}

const std::vector<CRectF>& CPDFPage::GetTextOrgMouseRects() const
{
	if(!m_optTextOrgMouseRects.has_value()){
		const auto& cursorRects = GetTextOrgCursorRects();
		auto mouseRects = GetTextCursorRects();
		if (!mouseRects.empty()) {
			for (std::size_t i = 0; i < mouseRects.size(); i++) {
				bool isFirst = i == 0;
				bool isLast = i == mouseRects.size() - 1;
				bool isAfterLF = !isFirst && is_lf(GetText()[i - 1]);
				bool isBeforeLast = i == mouseRects.size() - 2;
				bool isBeforeCR = !isLast && !isBeforeLast && is_cr(GetText()[i + 1]);
				bool isAfterSpace = !isFirst && is_space(GetText()[i - 1]);
				bool isBeforeSpace = !isLast && !isBeforeLast && is_space(GetText()[i + 1]);
				bool isSpaceMid = !isFirst && is_space(GetText()[i]);
				bool isCR = GetText()[i] == L'\r';
				bool isLF = GetText()[i] == L'\n';

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
		}
		m_optTextOrgMouseRects.emplace(mouseRects);
	}
	return m_optTextOrgMouseRects.value();
}

const std::vector<CRectF>& CPDFPage::GetTextMouseRects() const
{
	if (!m_optTextMouseRects.has_value()) {
		m_optTextMouseRects = GetTextOrgMouseRects();
		RotateRects(m_optTextMouseRects.value(), Rotate.get());
	}
	return m_optTextMouseRects.value();
}

const std::vector<CRectF>& CPDFPage::GetFindRects(const std::wstring& find_string)
{
	auto find = boost::trim_copy(find_string);
	if (!m_optFind.has_value() || m_optFind->Find != find) {
		std::vector<CRectF> rects;
		if (find.empty()) {
			m_optFind.emplace(find, rects);
		} else {
			auto results  = GetPDFiumPtr()->Text_FindResults(m_index, find);
			std::vector<CRectF> rects;
			for (const auto res : results) {
				rects.insert(rects.end(), std::get<2>(res).cbegin(), std::get<2>(res).cend());
			}
		//	FPDF_WIDESTRING text = reinterpret_cast<FPDF_WIDESTRING>(find.c_str());
		//	UNQ_FPDF_SCHHANDLE pSchHdl(GetPDFiumPtr()->Text_UnqFindStart(m_pTextPage.get(), text, 0, 0));
		//	while (GetPDFiumPtr()->Text_FindNext(pSchHdl.get())) {
		//		int index = GetPDFiumPtr()->Text_GetSchResultIndex(pSchHdl.get());
		//		int ch_count = GetPDFiumPtr()->Text_GetSchCount(pSchHdl.get());
		//		int rc_count = GetPDFiumPtr()->Text_CountRects(m_pTextPage.get(), index, ch_count);
		//		for (int i = 0; i < rc_count; i++) {
		//			double left, top, right, bottom;
		//			m_pDoc->GetPDFiumPtr()->Text_GetRect(
		//				m_pTextPage.get(),
		//				i,
		//				&left,
		//				&top,
		//				&right,
		//				&bottom);
		//			rects.emplace_back(
		//				static_cast<FLOAT>(left),
		//				static_cast<FLOAT>(top),
		//				static_cast<FLOAT>(right),
		//				static_cast<FLOAT>(bottom));
		//		}
		//	}
			RotateRects(rects, Rotate.get());
			m_optFind.emplace(find, rects);
		}
	}
	return m_optFind->FindRects;
}

CComPtr<ID2D1Bitmap1> CPDFPage::GetClipBitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int& rotate, const CRectF& rectInPage)
{
	return GetPDFiumPtr()->Bitmap_GetPageClippedBitmap(m_index, pDirect->GetHDC(), pDirect->GetD2DDeviceContext(), pDirect->GetWICImagingFactory(), rectInPage, scale);
}

CComPtr<ID2D1Bitmap1> CPDFPage::GetBitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int& rotate)
{
	return GetPDFiumPtr()->Bitmap_GetPageBitmap(m_index, pDirect->GetHDC(), pDirect->GetD2DDeviceContext(), pDirect->GetWICImagingFactory(), scale);
}

CRectF CPDFPage::GetCaretRect(const int index)
{
	if (!GetTextCursorRects().empty()) {
		auto rcInPdfiumPage = GetTextCursorRects()[index];
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
		return CRectF();
	}
}

CRectF CPDFPage::RotateRect(const CRectF& rc, const int& rotate) const
{
	CSizeF size = GetSize();
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

void CPDFPage::RotateRects(std::vector<CRectF>& rects, const int& rotate) const
{
	for (CRectF& rc : rects) {
		rc = RotateRect(rc, rotate);
	}
}

const std::vector<CRectF>& CPDFPage::GetSelectedTextRects(const int& begin, const int& end)
{
	if (!m_optSelectedText.has_value() || m_optSelectedText->Begin != begin || m_optSelectedText->End != end) {

		int rect_count = m_pDoc->GetPDFiumPtr()->Text_CountRects(m_index, begin, end - begin);
		std::vector<CRectF> rectsInPdfiumPage;
		for (auto i = 0; i < rect_count; i++) {
			double left, top, right, bottom;
			m_pDoc->GetPDFiumPtr()->Text_GetRect(
				m_index,
				i,
				&left,
				&top,
				&right,
				&bottom);
			rectsInPdfiumPage.emplace_back(
				static_cast<FLOAT>(left),
				static_cast<FLOAT>(top),
				static_cast<FLOAT>(right),
				static_cast<FLOAT>(bottom));
		}
		RotateRects(rectsInPdfiumPage, Rotate.get());
		m_optSelectedText.emplace(begin, end, rectsInPdfiumPage);
	}
	return m_optSelectedText->SelectedRects;
}

int CPDFPage::GetCursorCharIndexAtPos(const CPointF& ptInPdfiumPage)
{
	const auto& textMouseRects = GetTextMouseRects();
	auto iter = std::find_if(textMouseRects.cbegin(), textMouseRects.cend(),
		[ptInPdfiumPage](const CRectF& rc)->bool {
			return rc.left <= ptInPdfiumPage.x && ptInPdfiumPage.x < rc.right &&
				rc.top >= ptInPdfiumPage.y && ptInPdfiumPage.y > rc.bottom;
		});
	if (iter == textMouseRects.cend()) {
		return -1;
	} else {
		return std::distance(textMouseRects.cbegin(), iter);
	}
}



