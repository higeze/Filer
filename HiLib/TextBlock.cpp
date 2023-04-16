#include "TextBlock.h"
#include "D2DWWindow.h"

CSizeF CTextBlock::MeasureSize()
{
	return MeasureSize(m_text.get());
}

CSizeF CTextBlock::MeasureSize(const std::wstring& text)
{
	CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(
		m_spProperty->Format, text);
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
	CRectF rect = GetRectInWnd();
	rect.DeflateRect(m_spProperty->Padding);
	GetWndPtr()->GetDirectPtr()->DrawTextLayout(m_spProperty->Format, text, rect);

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}