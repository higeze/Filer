#include "UIElement.h"

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
		OnMouseEnter(e);
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

void CUIElement::OnMouseEnter(const MouseEvent& e)
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