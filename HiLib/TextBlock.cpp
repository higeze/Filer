#include "TextBlock.h"
#include "D2DWWindow.h"

void CTextBlock::Measure(const CSizeF& availableSize)
{
	Measure(availableSize, L"A");
}

void CTextBlock::Measure(const CSizeF& availableSize, const std::wstring& text)
{
	m_size = MeasureSize(Text->empty() ? text : *Text);
}

CSizeF CTextBlock::MeasureSize(const std::wstring& text)
{
	CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(GetFormat(), text);
	size.width += GetPadding().left
		+ GetPadding().right
		+ GetNormalBorder().Width;
	size.height += GetPadding().top
		+ GetPadding().bottom
		+ GetNormalBorder().Width;

	return size;
}

void CTextBlock::OnPaint(const PaintEvent& e)
{
	std::wstring text = *Text;

	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);
	CRectF rect = GetRectInWnd();
	rect.DeflateRect(GetPadding());
	GetWndPtr()->GetDirectPtr()->DrawTextLayout(GetFormat(), text, rect);

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}