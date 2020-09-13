#include "ProgressBar.h"
#include "D2DWWindow.h"
#include <boost/lexical_cast.hpp>

CProgressBar::CProgressBar(CD2DWControl* pParentControl, const std::shared_ptr<ProgressProperty>& spProgressProp)
	:CD2DWControl(pParentControl), m_spStatusBarProp(spProgressProp){}

void CProgressBar::OnPaint(const PaintEvent& e)
{
	//Draw background
	CRectF backRect(GetRectInWnd());
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->BackgroundFill, backRect);

	//Draw foreground
	CRectF foreRect(
		backRect.left,
		backRect.top,
		backRect.left + backRect.Width()*GetValue() / (GetMax() - GetMin()),
		backRect.bottom);
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->ForegroundFill, foreRect);

	//Draw border
	GetWndPtr()->GetDirectPtr()->DrawSolidRectangle(m_spStatusBarProp->Border, backRect);

	//Draw text
	std::wstring text = boost::lexical_cast<std::wstring>(GetValue()) + L" /  " + boost::lexical_cast<std::wstring>(GetMax() - GetMin());
	GetWndPtr()->GetDirectPtr()->DrawTextInRect(m_spStatusBarProp->Format, text, backRect);
}
