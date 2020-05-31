#include "ProgressBar.h"
#include <boost/lexical_cast.hpp>

namespace d2dw
{
	CProgressBar::CProgressBar(CWnd* pWnd, const std::shared_ptr<ProgressProperty>& spProgressProp)
		:CUIElement(), m_pWnd(pWnd), m_spStatusBarProp(spProgressProp){}

	void CProgressBar::OnPaint(const PaintEvent& e)
	{
		//Draw background
		CRectF backRect(GetRect());
		e.WndPtr->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->BackgroundFill, backRect);

		//Draw foreground
		CRectF foreRect(
			backRect.left,
			backRect.top,
			backRect.left + backRect.Width()*GetValue() / (GetMax() - GetMin()),
			backRect.bottom);
		e.WndPtr->GetDirectPtr()->FillSolidRectangle(m_spStatusBarProp->ForegroundFill, foreRect);

		//Draw border
		e.WndPtr->GetDirectPtr()->DrawSolidRectangle(m_spStatusBarProp->Border, backRect);

		//Draw text
		std::wstring text = boost::lexical_cast<std::wstring>(GetValue()) + L" /  " + boost::lexical_cast<std::wstring>(GetMax() - GetMin());
		e.WndPtr->GetDirectPtr()->DrawTextInRect(m_spStatusBarProp->Format, text, backRect);
	}
}
