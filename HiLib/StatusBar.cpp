#include "StatusBar.h"
#include "Debug.h"
#include "D2DWWindow.h"

CStatusBar::CStatusBar(CD2DWControl* pParentControl, const std::shared_ptr<StatusBarProperty>& spStatusBarProp)
	:CD2DWControl(pParentControl),
	m_spStatusBarProp(spStatusBarProp)
{}

CStatusBar::~CStatusBar() = default;

void CStatusBar::OnPaint(const PaintEvent& e)
{
	auto rcPaint = GetRectInWnd();
	if (rcPaint.Width() > 0 && rcPaint.Height() > 0) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->BackgroundFill, rcPaint);
		GetWndPtr()->GetDirectPtr()->DrawTextLayout(m_spStatusBarProp->Format, m_text, rcPaint);
	}
}

CSizeF CStatusBar::MeasureSize(const std::wstring& text)
{
	CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(
		m_spStatusBarProp->Format, text);
	size.width += m_spStatusBarProp->Padding.top
		+ m_spStatusBarProp->Padding.bottom;
	size.height += m_spStatusBarProp->Padding.left
		+ m_spStatusBarProp->Padding.right;
	return size;
}