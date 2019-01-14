#include "Celler.h"
#include "Cell.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"

void CCeller::OnLButtonDown(CSheet* pSheet, const LButtonDownEvent& e)
{
	auto cell = pSheet->Cell(e.Direct.Pixels2Dips(e.Point));
	if (cell != nullptr) {
		cell->OnLButtonDown(e);
	}
}
void CCeller::OnLButtonUp(CSheet* pSheet, const LButtonUpEvent& e)
{
	auto cell = pSheet->Cell(e.Direct.Pixels2Dips(e.Point));
	if (cell != nullptr) {
		cell->OnLButtonUp(e);
	}
}
void CCeller::OnLButtonClk(CSheet* pSheet, const LButtonClkEvent& e)
{
	auto cell = pSheet->Cell(e.Direct.Pixels2Dips(e.Point));
	if (cell && (cell->GetState() == UIElementState::Pressed || cell->GetState() == UIElementState::PressedLeave)) {
		cell->OnLButtonClk(e);
	}
}
void CCeller::OnLButtonSnglClk(CSheet* pSheet, const LButtonSnglClkEvent& e)
{
	auto cell = pSheet->Cell(e.Direct.Pixels2Dips(e.Point));
	if (cell != nullptr) {
		cell->OnLButtonSnglClk(e);
	}
}
void CCeller::OnLButtonDblClk(CSheet* pSheet, const LButtonDblClkEvent& e)
{
	auto cell = pSheet->Cell(e.Direct.Pixels2Dips(e.Point));
	if (cell != nullptr) {
		cell->OnLButtonDblClk(e);
	}
	else {
		pSheet->OnBkGndLButtondDblClk(e);
	}
}
void CCeller::OnMouseMove(CSheet* pSheet, const MouseMoveEvent& e)
{
	auto cell = pSheet->Cell(e.Direct.Pixels2Dips(e.Point));
	if (m_cellUnderMouse != nullptr && cell.get() != m_cellUnderMouse.get()) {
		m_cellUnderMouse->OnMouseLeave(MouseLeaveEvent(e.WindowPtr, e.Direct, e.Flags, e.Point));
	}
	m_cellUnderMouse = cell;
	if (cell != nullptr) {
		cell->OnMouseMove(e);
	}
}

void CCeller::OnMouseLeave(CSheet* pSheet, const MouseLeaveEvent& e)
{
	auto cell = pSheet->Cell(e.Direct.Pixels2Dips(e.Point));
	if (m_cellUnderMouse != nullptr) {
		pSheet->UnhotAll();//TODO
		m_cellUnderMouse->OnMouseLeave(e);
	}
	m_cellUnderMouse = cell;
}


void CCeller::OnContextMenu(CSheet* pSheet, const ContextMenuEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetGridPtr()->GetDirect()->Pixels2Dips(e.Point));
	if (cell != nullptr) {
		cell->OnContextMenu(e);
	}
}
//void CCeller::OnSetFocus(CSheet* pSheet, const EventArgs& e){}
//void CCeller::OnKillFocus(CSheet* pSheet, const EventArgs& e){}
void CCeller::OnSetCursor(CSheet* pSheet, const SetCursorEvent& e)
{
	CPoint pt;
	::GetCursorPos(&pt);
	e.WindowPtr->ScreenToClient(&pt);
	auto cell = pSheet->Cell(pSheet->GetGridPtr()->GetDirect()->Pixels2Dips(pt));
	if (cell != nullptr) {
		cell->OnSetCursor(e);
	}
}

void CCeller::OnKeyDown(CSheet* pSheet, const KeyDownEvent& e)
{
	//TODO Combine with Curosr
}

void CCeller::Clear()
{
	m_cellUnderMouse.reset();
}