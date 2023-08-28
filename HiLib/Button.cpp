#include "Button.h"
#include "ButtonProperty.h"
#include "D2DWWindow.h"

void CButton::OnLButtonDown(const LButtonDownEvent& e)
{
	CUIElement::OnLButtonDown(e);
}

void CButton::OnLButtonUp(const LButtonUpEvent& e)
{
	if (GetState() == UIElementState::Pressed) {
		Command.execute();
	}
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
	FLOAT radius = GetRectInWnd().Height() * 0.1f;
	SolidFill fill = m_spButtonProperty->NormalFill;
	SolidLine line = m_spButtonProperty->BorderLine;
	FormatF format = m_spButtonProperty->Format;
	std::wstring content = *Content;
	if (*IsEnabled) {
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

		if (GetIsFocused()) {
			line = m_spButtonProperty->FocusedLine;
		}
	} else {
		format = m_spButtonProperty->DisableFormat;
		if (!DisableContent->empty()) {
			content = *DisableContent;
		}
	}

	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRoundedRectangle(fill, GetRectInWnd(), radius, radius);
	GetWndPtr()->GetDirectPtr()->DrawSolidRoundedRectangle(line, GetRectInWnd(), radius, radius);
	GetWndPtr()->GetDirectPtr()->DrawTextInRect(format, content, GetRectInWnd());

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CButton::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
}

void CButton::OnKeyDown(const KeyDownEvent& e)
{
	if (e.Char == VK_RETURN) {
		Command.execute();
		*e.HandledPtr = TRUE;
	}
}
