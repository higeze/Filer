#pragma once
#include <tuple>
#include "Direct2DWrite.h"
#include "Timer.h"

class CPdfView;
class CPdfDoc;

class CPDFCaret
{
private:
	bool m_bCaret = true;
	CTimer m_timer;
	CPdfView* m_pControl;
public:
	CPDFCaret(CPdfView* pControl) :
		m_pControl(pControl){}
	std::tuple<int, int> Old = { 0, 0 };
	std::tuple<int, int> Current = { 0, 0 };
	std::tuple<int, int> Anchor = { 0, 0 };
	std::tuple<int, int> SelectBegin = { 0, 0 };
	std::tuple<int, int> SelectEnd = { 0, 0 };
	std::tuple<int, CPointF> Point = { 0, { 0.f, 0.f } };
	bool IsCaret()const { return m_bCaret; }

	void Move(const int page_index, const int& char_index, const CPointF& point);
	void MoveWithShift(const int page_index, const int& char_index, const CPointF& point);
	void MoveSelection(const int sel_begin_page, const int& sel_begin_char, const int sel_end_page, const int& sel_end_char);
	void StartBlink();
	void StopBlink();

};