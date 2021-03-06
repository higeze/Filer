#include "ProgressBar.h"
#include <boost/lexical_cast.hpp>

namespace d2dw
{
	CProgressBar::CProgressBar(CWnd* pWnd, const std::shared_ptr<ProgressProperty>& spProgressProp)
		:CUIElement(), m_pWnd(pWnd), m_spProgressProp(spProgressProp){}

	void CProgressBar::OnPaint(const PaintEvent& e)
	{
		//Draw background
		CRectF backRect(GetRect());
		e.Direct.FillSolidRectangle(m_spProgressProp->BackgroundFill, backRect);

		//Draw foreground
		CRectF foreRect(
			backRect.left,
			backRect.top,
			backRect.left + backRect.Width()*GetValue() / (GetMax() - GetMin()),
			backRect.bottom);
		e.Direct.FillSolidRectangle(m_spProgressProp->ForegroundFill, foreRect);

		//Draw border
		e.Direct.DrawSolidRectangle(m_spProgressProp->Border, backRect);

		//Draw text
		std::wstring text = boost::lexical_cast<std::wstring>(GetValue()) + L" /  " + boost::lexical_cast<std::wstring>(GetMax() - GetMin());
		e.Direct.DrawTextInRect(m_spProgressProp->Format, text, backRect);
	}
}
