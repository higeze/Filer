#include "StatusBar.h"
#include "Debug.h"
#include "D2DWWindow.h"

CStatusBar::CStatusBar(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl){}

CStatusBar::~CStatusBar() = default;

void CStatusBar::Measure(const CSizeF& availableSize)
{
	m_size = MeasureSize(L"A");
}

void CStatusBar::OnPaint(const PaintEvent& e)
{
	auto rcPaint = GetRectInWnd();
	if (rcPaint.Width() > 0 && rcPaint.Height() > 0) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), rcPaint);
		GetWndPtr()->GetDirectPtr()->DrawTextLayout(GetFormat(), *Text, rcPaint);
	}
}

CSizeF CStatusBar::MeasureSize(const std::wstring& text)
{
	CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(
		GetFormat(), text);
	size.width += GetPadding().top
		+ GetPadding().bottom;
	size.height += GetPadding().left
		+ GetPadding().right;
	return size;
}