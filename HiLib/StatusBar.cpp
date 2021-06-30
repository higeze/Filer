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
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->BackgroundFill, rcPaint);
	GetWndPtr()->GetDirectPtr()->DrawTextLayout(m_spStatusBarProp->Format, m_text, rcPaint);
}

CSizeF CStatusBar::MeasureSize(CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	std::wstring text;
	if (text.empty()) { text = L"a"; }
	return pDirect->CalcTextSize(m_spStatusBarProp->Format, text);
}