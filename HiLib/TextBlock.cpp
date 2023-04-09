#include "TextBlock.h"
#include "D2DWWindow.h"

CSizeF CTextBlock::MeasureSize()
{
	CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(
		m_spProperty->Format, GetText().get());
	size.width += m_spProperty->Padding.top
		+ m_spProperty->Padding.bottom;
	size.height += m_spProperty->Padding.left
		+ m_spProperty->Padding.right;
	return size;
}

void CTextBlock::OnPaint(const PaintEvent& e)
{
	std::wstring text = m_text.get();

	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->DrawTextInRect(m_spProperty->Format, text, GetRectInWnd());

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}