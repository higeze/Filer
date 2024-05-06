#include "TextBlock.h"
#include "D2DWWindow.h"

CSizeF CTextBlock::MeasureSize()
{
	return MeasureSize(*Text);
}

CSizeF CTextBlock::MeasureSize(const std::wstring& text)
{
	CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(
		GetFormat(), text);
	size.width += GetPadding().top
		+ GetPadding().bottom;
	size.height += GetPadding().left
		+ GetPadding().right;
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