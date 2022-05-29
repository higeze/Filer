#include "PDFCaret.h"
#include "PdfView.h"

void CPDFCaret::Move(const int page_index, const int& char_index, const CPointF& point)
{
	auto index = std::make_tuple(page_index, char_index);
	Old = Current;
	Current = index;
	Anchor =  index;
	SelectBegin = index;
	SelectEnd = index;
	Point = std::make_tuple(page_index, point);

	StartBlink();
}
void CPDFCaret::MoveWithShift(const int page_index, const int& char_index, const CPointF& point)
{
	auto compare = [](const std::tuple<int, int>& lhs, const std::tuple<int, int>& rhs)->int
	{
		if (lhs == rhs) {
			return 0;
		} else if (std::get<0>(lhs) < std::get<0>(rhs)) {
			return -1;
		} else if (std::get<0>(lhs) > std::get<0>(rhs)) {
			return 1;
		} else {
			if (std::get<1>(lhs) < std::get<1>(rhs)) {
				return -1;
			} else if (std::get<1>(lhs) > std::get<1>(rhs)) {
				return 1;
			} else {
				return 0;
			}
		}
	};

	auto index = std::make_tuple(page_index, char_index);
	Old = Current;
	Current = std::make_tuple(page_index, char_index);
	//Anchor
	SelectBegin = compare(index, Anchor) < 0 ? index : Anchor;
	SelectEnd = compare(index, Anchor) > 0 ? index : Anchor;
	Point = std::make_tuple(page_index, point);

	StartBlink();
}
void CPDFCaret::MoveSelection(const int sel_begin_page, const int& sel_begin_char, const int sel_end_page, const int& sel_end_char)
{
	auto begin_index = std::make_tuple(sel_begin_page, sel_begin_char);
	auto end_index = std::make_tuple(sel_end_page, sel_end_char);

	Old = Current;
	Current = end_index;
	Anchor = begin_index;
	SelectBegin = begin_index;
	SelectEnd = end_index;
	//Point = std::make_tuple(page_index, point);

	StartBlink();
}

void CPDFCaret::Clear()
{
	bool m_bCaret = true;
	m_timer.stop();
	Old = { 0, 0 };
	Current = { 0, 0 };
	Anchor = { 0, 0 };
	SelectBegin = { 0, 0 };
	SelectEnd = { 0, 0 };
	Point = { 0, { 0.f, 0.f } };
}

void CPDFCaret::StartBlink()
{
	m_bCaret = true;
	m_timer.run([this]()->void
		{
			m_bCaret = !m_bCaret;
			m_pControl->GetWndPtr()->InvalidateRect(NULL, FALSE);
		}, 
		std::chrono::milliseconds(::GetCaretBlinkTime()));
	m_pControl->GetWndPtr()->InvalidateRect(NULL, FALSE);
}

void CPDFCaret::StopBlink()
{
	m_bCaret = false;
	m_timer.stop();
}