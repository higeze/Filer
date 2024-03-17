#include "Celler.h"
#include "Cursorer.h"
#include "Cell.h"
#include "GridView.h"
#include "GridView.h"
#include "SheetEventArgs.h"
#include "D2DWWindow.h"

void CCeller::OnLButtonDown(CGridView* pSheet, const LButtonDownEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonDown(e);
	}
}
void CCeller::OnLButtonUp(CGridView* pSheet, const LButtonUpEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonUp(e);
	}
}
void CCeller::OnLButtonClk(CGridView* pSheet, const LButtonClkEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell && (cell->GetState() == UIElementState::Pressed || cell->GetState() == UIElementState::PressedLeave)) {
		cell->OnLButtonClk(e);
	}
}
void CCeller::OnLButtonSnglClk(CGridView* pSheet, const LButtonSnglClkEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonSnglClk(e);
	}
}
void CCeller::OnLButtonDblClk(CGridView* pSheet, const LButtonDblClkEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonDblClk(e);
	}
	else {
		pSheet->OnBkGndLButtondDblClk(e);
	}
}
void CCeller::OnLButtonBeginDrag(CGridView* pSheet, const LButtonBeginDragEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnLButtonBeginDrag(e);
	} else {
		
	}
}

#include "ToolTip.h"
#include "MyIcon.h"

void CCeller::OnMouseMove(CGridView* pSheet, const MouseMoveEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));

	//auto InitialShowDelay = ::GetDoubleClickTime();
	//auto BetweenShowDelay = InitialShowDelay / 5;
	//auto ShowDuration = InitialShowDelay * 10;

	//auto GetCursorSize = []()->CSize {
	//	return CSize(::GetSystemMetrics(SM_CXCURSOR), ::GetSystemMetrics(SM_CYCURSOR));
	//};

	//auto GetCursorIconSize = []()->CSize {
	//	CIcon icon(::GetCursor());
	//	CSize iconSize;
	//	ICONINFO ii;
	//	if (::GetIconInfo(icon, &ii)) {
	//		if (BITMAP bm; ::GetObjectW(ii.hbmMask, sizeof(bm), &bm) == sizeof(bm)) {
	//			iconSize.cx = bm.bmWidth;
	//			iconSize.cy = ii.hbmColor ? bm.bmHeight : bm.bmHeight / 2;
	//		}
	//		if (ii.hbmMask)  ::DeleteObject(ii.hbmMask);
	//		if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
	//	}
	//	return iconSize;
	//};

	//auto leave = cell == nullptr;
	//auto initialEnter = !leave && m_cellUnderMouse == nullptr && cell != nullptr;
	//auto betweenEnter = !leave && !initialEnter && m_cellUnderMouse != cell;


	//if (leave) {
	//	if (pSheet->GetWndPtr()->GetToolTipControlPtr()) {
	//		pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
	//	}
	//} else if (initialEnter || betweenEnter) {
	//	auto delay = initialEnter ? InitialShowDelay : BetweenShowDelay;
	//	if (pSheet->GetWndPtr()->GetToolTipControlPtr()) {
	//		pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
	//	}

	//	m_deadlinetimer.run([pSheet, e, cell, sz = GetCursorSize()]()->void
	//	{
	//		auto spTT = std::make_shared<CToolTip>(
	//			pSheet,
	//			std::make_shared<ToolTipProperty>());
	//		CPointF pt = e.WndPtr->GetCursorPosInWnd();
	//		pt.x += sz.cx/2;
	//		spTT->OnCreate(CreateEvt(pSheet->GetWndPtr(), pSheet->GetWndPtr(), CRectF()));
	//		spTT->Content.set(cell->GetString());
	//		spTT->Measure(CSizeF(FLT_MAX, FLT_MAX));
	//		spTT->Arrange(CRectF(pt, spTT->DesiredSize()));

	//	}, std::chrono::milliseconds(delay));
	//} 

	if (m_cellUnderMouse != cell) {
		if (m_cellUnderMouse != nullptr) {
			m_cellUnderMouse->OnMouseLeave(MouseLeaveEvent(pSheet->GetWndPtr(), e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		}
		if (cell != nullptr) {
			::OutputDebugStringA(std::format("{}\r\n", (LONG)cell.get()).c_str());
			cell->OnMouseEnter(MouseEnterEvent(pSheet->GetWndPtr(), e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		}
		m_cellUnderMouse = cell;
	}

	if (cell != nullptr) {
		cell->OnMouseMove(e);
	}
}

void CCeller::OnMouseLeave(CGridView* pSheet, const MouseLeaveEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (m_cellUnderMouse != nullptr) {
		pSheet->UnhotAll();//TODO
		m_cellUnderMouse->OnMouseLeave(e);
	}
	m_cellUnderMouse = cell;
}


void CCeller::OnContextMenu(CGridView* pSheet, const ContextMenuEvent& e)
{
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (cell != nullptr) {
		cell->OnContextMenu(e);
	}
}
//void CCeller::OnSetFocus(CGridView* pSheet, const EventArgs& e){}
//void CCeller::OnKillFocus(CGridView* pSheet, const EventArgs& e){}
void CCeller::OnSetCursor(CGridView* pSheet, const SetCursorEvent& e)
{
	CPoint pt = pSheet->GetWndPtr()->GetCursorPosInClient();
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(pt));
	if (cell != nullptr) {
		cell->OnSetCursor(e);
	} else {
		*e.HandledPtr = FALSE;
	}
}

void CCeller::OnKeyDown(CGridView* pSheet, const KeyDownEvent& e)
{
	auto cell = pSheet->GetCursorerPtr()->GetFocusedCell();
	if (cell != nullptr) {
		cell->OnKeyDown(e);
	}
}

void CCeller::OnKeyTraceDown(CGridView* pSheet, const KeyTraceDownEvent& e)
{
	auto cell = pSheet->GetCursorerPtr()->GetFocusedCell();
	if (cell != nullptr) {
		cell->OnKeyTraceDown(e);
	}
}
void CCeller::OnChar(CGridView* pSheet, const CharEvent& e)
{
	auto cell = pSheet->GetCursorerPtr()->GetFocusedCell();
	if (cell != nullptr) {
		cell->OnChar(e);
	}
}

void CCeller::OnImeStartComposition(CGridView* pSheet, const ImeStartCompositionEvent& e)
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