#include "Celler.h"
#include "Cursorer.h"
#include "Cell.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"
#include "D2DWWindow.h"

void CCeller::OnLButtonDown(CSheet* pSheet, const LButtonDownEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonDown(e);
	}
}
void CCeller::OnLButtonUp(CSheet* pSheet, const LButtonUpEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonUp(e);
	}
}
void CCeller::OnLButtonClk(CSheet* pSheet, const LButtonClkEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell && (cell->GetState() == UIElementState::Pressed || cell->GetState() == UIElementState::PressedLeave)) {
		cell->OnLButtonClk(e);
	}
}
void CCeller::OnLButtonSnglClk(CSheet* pSheet, const LButtonSnglClkEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonSnglClk(e);
	}
}
void CCeller::OnLButtonDblClk(CSheet* pSheet, const LButtonDblClkEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonDblClk(e);
	}
	else {
		pSheet->OnBkGndLButtondDblClk(e);
	}
}
void CCeller::OnLButtonBeginDrag(CSheet* pSheet, const LButtonBeginDragEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonBeginDrag(e);
	} else {
		
	}
}

void CCeller::OnMouseMove(CSheet* pSheet, const MouseMoveEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (m_cellUnderMouse != nullptr && cell.get() != m_cellUnderMouse.get()) {
		m_cellUnderMouse->OnMouseLeave(MouseLeaveEvent(pSheet->GetWndPtr(), e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
	}
	m_cellUnderMouse = cell;
	if (cell != nullptr) {
		cell->OnMouseMove(e);
	}
}

void CCeller::OnMouseLeave(CSheet* pSheet, const MouseLeaveEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (m_cellUnderMouse != nullptr) {
		pSheet->UnhotAll();//TODO
		m_cellUnderMouse->OnMouseLeave(e);
	}
	m_cellUnderMouse = cell;
}


void CCeller::OnContextMenu(CSheet* pSheet, const ContextMenuEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnContextMenu(e);
	}
}
//void CCeller::OnSetFocus(CSheet* pSheet, const EventArgs& e){}
//void CCeller::OnKillFocus(CSheet* pSheet, const EventArgs& e){}
void CCeller::OnSetCursor(CSheet* pSheet, const SetCursorEvent& e)
{
	CPoint pt = pSheet->GetWndPtr()->GetCursorPosInClient();
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(pt));
	if (cell != nullptr) {
		cell->OnSetCursor(e);
	} else {
		*e.HandledPtr = FALSE;
	}
}

void CCeller::OnKeyDown(CSheet* pSheet, const KeyDownEvent& e)
{
	auto cell = pSheet->GetCursorerPtr()->GetFocusedCell();
	if (cell != nullptr) {
		cell->OnKeyDown(e);
	}
}

void CCeller::OnChar(CSheet* pSheet, const CharEvent& e)
{
	auto cell = pSheet->GetCursorerPtr()->GetFocusedCell();
	if (cell != nullptr) {
		cell->OnChar(e);
	}
}

void CCeller::OnImeStartComposition(CSheet* pSheet, const ImeStartCompositionEvent& e)
{
	auto cell = pSheet->GetCursorerPtr()->GetFocusedCell();
	if (cell != nullptr) {
		cell->OnImeStartComposition(e);
	}
}


void CCeller::OnClear()
{
	Clear();
}

void CCeller::Clear()
{
	m_cellUnderMouse.reset();
}