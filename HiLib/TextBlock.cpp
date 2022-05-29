#include "TextBlock.h"
#include "D2DWWindow.h"

void CTextBlock::OnPaint(const PaintEvent& e)
{
	std::wstring text = m_text.get();

	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->DrawTextInRect(m_spProperty->Format, text, GetRectInWnd());

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}