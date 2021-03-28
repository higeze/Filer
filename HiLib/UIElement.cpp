#include "UIElement.h"
#include "D2DWWindow.h"
#include <windowsx.h>

CreateEvt::CreateEvt(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled)
	:Event(pWnd, pHandled), Rect()
{
	auto pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
	Rect = CRect(pCS->x, pCS->y, pCS->x + pCS->cx, pCS->y + pCS->cy);
	RectF = pWnd->GetDirectPtr()->Pixels2Dips(Rect);
}

ContextMenuEvent::ContextMenuEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled)
	:Event(pWnd, pHandled),
	PointInScreen(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)),
	PointInClient(pWnd->ScreenToClient(CPoint((short)LOWORD(lParam), (short)HIWORD(lParam))).value()),
	PointInWnd(pWnd->GetDirectPtr()->Pixels2Dips(PointInClient)){}

MouseEvent::MouseEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled)
		:Event(pWnd, pHandled), Flags(wParam),
		PointInClient(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)),
		PointInWnd(pWnd->GetDirectPtr()->Pixels2Dips(PointInClient)){}

SetCursorEvent::SetCursorEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled)
	:Event(pWnd, pHandled),
	HitTest(LOWORD(lParam)),
	PointInWnd(pWnd->GetDirectPtr()->Pixels2Dips(pWnd->GetCursorPosInClient())){}

MouseWheelEvent::MouseWheelEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled)
	:Event(pWnd, pHandled),
	PointInScreen(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)),
	PointInClient(pWnd->ScreenToClient(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))).value()),
	PointInWnd(pWnd->GetDirectPtr()->Pixels2Dips(PointInClient)),
	Delta(GET_WHEEL_DELTA_WPARAM(wParam)){}




void CUIElement::SetState(const UIElementState::Type& state)
{
	if (m_state != state) {
		m_state = state;
		OnPropertyChanged(L"state");
	}
}

void CUIElement::OnLButtonDown(const LButtonDownEvent& e)
{
	SetState(UIElementState::Pressed);
}

void CUIElement::OnLButtonUp(const LButtonUpEvent& e)
{
	SetState(UIElementState::Hot);
}

void CUIElement::OnMouseMove(const MouseMoveEvent& e)
{
	switch(m_state)
	{
	//Not in element
	case UIElementState::None:
	case UIElementState::Normal:
	case UIElementState::PressedLeave:
		OnMouseEnter(MouseEnterEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		break;
	//Already in element
	case UIElementState::Hot:
	case UIElementState::Pressed:
		break;
	//Disabled
	case UIElementState::Disabled:
		break;
	}
	//if((m_state==UIElementState::Pressed || m_state==UIElementState::PressedLeave) && e.Flags==MK_LBUTTON){
	//	SetState(UIElementState::Pressed);
	//}else{
	//	SetState(UIElementState::Hot);
	//}
}

void CUIElement::OnMouseEnter(const MouseEnterEvent& e)
{
	if(m_state==UIElementState::PressedLeave && e.Flags==MK_LBUTTON){
		SetState(UIElementState::Pressed);
	}else{
		SetState(UIElementState::Hot);
	}
}

void CUIElement::OnMouseLeave(const MouseLeaveEvent& e)
{

	if(m_state==UIElementState::Pressed && e.Flags==MK_LBUTTON){
		SetState(UIElementState::PressedLeave);	
	}else{
		SetState(UIElementState::Normal);
	}
}