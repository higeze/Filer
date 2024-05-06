#include "Button.h"
#include "ButtonProperty.h"
#include "D2DWWindow.h"

void CButton::Measure(const CSizeF& availableSize)
{
	if (!m_opt_size.has_value()) {
		CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(GetFormat(), *Content);
		size.width += GetPadding().top
			+ GetPadding().bottom
			+ GetNormalBorder().Width * 2;
		size.height += GetPadding().left
			+ GetPadding().right
			+ GetNormalBorder().Width * 2;
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
	CRectF renderRect = RenderRect();
	FLOAT radius = renderRect.Height() * 0.1f;
	SolidFill fill = GetNormalBackground();
	SolidLine line = GetNormalBorder();
	FormatF format = GetFormat();
	std::wstring content = *Content;
	if (*IsEnabled) {
		switch (GetState()) {
			case UIElementState::Hot:
				fill = GetHotOverlay();
				break;
			case UIElementState::Pressed:
				fill = GetPressedBackground();
				break;
			case UIElementState::Normal:
			default:
				break;
		}

	} else {
		format = GetDisableFormat();
		if (!DisableContent->empty()) {
			content = *DisableContent;
		}
	}

	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(renderRect, D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRoundedRectangle(fill, renderRect, radius, radius);
	GetWndPtr()->GetDirectPtr()->DrawSolidRoundedRectangle(line, renderRect, radius, radius);
	GetWndPtr()->GetDirectPtr()->DrawTextInRect(format, content, renderRect);

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CButton::OnKeyDown(const KeyDownEvent& e)
{
	if (e.Char == VK_RETURN) {
		Command.execute();
		*e.HandledPtr = TRUE;
	}
}
