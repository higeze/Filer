#include "Button.h"
#include "D2DWWindow.h"

void CButton::OnLButtonDown(const LButtonDownEvent& e)
{
	CUIElement::OnLButtonDown(e);
}

void CButton::OnLButtonUp(const LButtonUpEvent& e)
{
	CUIElement::OnLButtonUp(e);
}

void CButton::OnMouseMove(const MouseMoveEvent& e)
{
	CUIElement::OnMouseMove(e);
}

void CButton::OnMouseEnter(const MouseEnterEvent& e)
{
	CUIElement::OnMouseEnter(e);
}

void CButton::OnMouseLeave(const MouseLeaveEvent& e)
{
	CUIElement::OnMouseLeave(e);
}

void CButton::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	FLOAT radius = GetRectInWnd().Height() * 0.1f;
	SolidFill fill = m_spButtonProperty->NormalFill;
	switch (GetState()) {
		case UIElementState::Hot:
			fill = m_spButtonProperty->HotFill;
			break;
		case UIElementState::Pressed:
			fill = m_spButtonProperty->PressedFill;
			break;
		case UIElementState::Normal:
		default:
			break;
	}
	GetWndPtr()->GetDirectPtr()->FillSolidRoundedRectangle(fill, GetRectInWnd(), radius, radius);
	GetWndPtr()->GetDirectPtr()->DrawSolidRoundedRectangle(m_spButtonProperty->BorderLine, GetRectInWnd(), radius, radius);
	GetWndPtr()->GetDirectPtr()->DrawTextInRect(m_spButtonProperty->Format, m_content.get(), GetRectInWnd());

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CButton::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
	GetCommand().Execute();
}

void CButton::OnKeyDown(const KeyDownEvent& e)
{
	if (e.Char == VK_RETURN) {
		GetCommand().Execute();
		*e.HandledPtr = TRUE;
	}
}
