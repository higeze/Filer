#include "Button.h"
#include "ButtonProperty.h"
#include "D2DWWindow.h"

void CButton::Measure(const CSizeF& availableSize)
{
	if (!m_opt_size.has_value()) {
		CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(m_spButtonProperty->Format, *Content);
		size.width += m_spButtonProperty->Padding.top
			+ m_spButtonProperty->Padding.bottom
			+ m_spButtonProperty->BorderLine.Width * 2;
		size.height += m_spButtonProperty->Padding.left
			+ m_spButtonProperty->Padding.right
			+ m_spButtonProperty->BorderLine.Width * 2;
		m_opt_size.emplace(size);
	}
}
CSizeF CButton::DesiredSize() const
{
	return m_opt_size.value();
}
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

void CButton::OnKeyDown(const KeyDownEvent& e)
{
	if (e.Char == VK_RETURN) {
		Command.execute();
		*e.HandledPtr = TRUE;
	}
}
