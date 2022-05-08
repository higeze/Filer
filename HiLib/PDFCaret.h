#pragma once
#include <tuple>
#include "Direct2DWrite.h"
class CPdfView;
class CPdfDoc;

class CPDFCaret
{
	CPDFCaret(CPdfView* pControl, CPdfDoc* pDoc);
	std::tuple<int, int> Old = { 0, 0 };
	std::tuple<int, int> Current = { 0, 0 };
	std::tuple<int, int> Anchor = { 0, 0 };
	std::tuple<int, int> SelectBegin = { 0, 0 };
	std::tuple<int, int> SelectEnd = { 0, 0 };
	std::tuple<int, CPointF> Point = { 0, { 0.f, 0.f } };

	void Move(const int page_index, const int& char_index, const CPointF& point);
	void MoveWithShift(const int page_index, const int& char_index, const CPointF& point);
	void MoveSelection(const int sel_begin_page, const int& sel_begin_char, const int sel_end_page, const int& sel_end_char);
	void StartBlink() {}
};