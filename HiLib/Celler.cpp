#include "Celler.h"
#include "Cell.h"
#include "Sheet.h"
#include "SheetEventArgs.h"

void CCeller::OnLButtonDown(CSheet* pSheet, const MouseEventArgs& e)
{
	auto cell = pSheet->Cell(e.Point);
	if (cell != nullptr) {
		cell->OnLButtonDown((MouseEventArgs)e);
	}
}
void CCeller::OnLButtonUp(CSheet* pSheet, const MouseEventArgs& e)
{
	auto cell = pSheet->Cell(e.Point);
	if (cell != nullptr) {
		cell->OnLButtonUp((MouseEventArgs)e);
	}
}
void CCeller::OnLButtonSnglClk(CSheet* pSheet, const MouseEventArgs& e)
{
	auto cell = pSheet->Cell(e.Point);
	if (cell != nullptr) {
		cell->OnLButtonSnglClk((MouseEventArgs)e);
	}
}
void CCeller::OnLButtonDblClk(CSheet* pSheet, const MouseEventArgs& e)
{
	auto cell = pSheet->Cell(e.Point);
	if (cell != nullptr) {
		cell->OnLButtonDblClk((MouseEventArgs)e);
	}
	else {
		pSheet->OnBkGndLButtondDblClk(e);
	}
}
void CCeller::OnMouseMove(CSheet* pSheet, const MouseEventArgs& e)
{
	auto cell = pSheet->Cell(e.Point);
	if (m_cellUnderMouse != nullptr && cell.get() != m_cellUnderMouse.get()) {
		m_cellUnderMouse->OnMouseLeave((MouseEventArgs)e);
	}
	m_cellUnderMouse = cell;
	if (cell != nullptr) {
		cell->OnMouseMove((MouseEventArgs)e);
	}
}

void CCeller::OnMouseLeave(CSheet* pSheet, const MouseEventArgs& e)
{
	auto cell = pSheet->Cell(e.Point);
	if (m_cellUnderMouse != nullptr) {
		pSheet->UnhotAll();//TODO
		m_cellUnderMouse->OnMouseLeave((MouseEventArgs)e);
	}
	m_cellUnderMouse = cell;
}


void CCeller::OnContextMenu(CSheet* pSheet, const ContextMenuEventArgs& e)
{
	auto cell = pSheet->Cell(e.Point);
	if (cell != nullptr) {
		cell->OnContextMenu((ContextMenuEventArgs)e);
	}
}
//void CCeller::OnSetFocus(CSheet* pSheet, const EventArgs& e){}
//void CCeller::OnKillFocus(CSheet* pSheet, const EventArgs& e){}
void CCeller::OnSetCursor(CSheet* pSheet, const SetCursorEventArgs& e)
{
	CPoint pt;
	::GetCursorPos(&pt);
	::ScreenToClient(e.HWnd, &pt);
	auto cell = pSheet->Cell(pt);
	if (cell != nullptr) {
		cell->OnSetCursor((SetCursorEventArgs)e);
	}
}
void CCeller::OnKeyDown(CSheet* pSheet, const KeyEventArgs& e)
{
	//TODO Combine with Curosr
}