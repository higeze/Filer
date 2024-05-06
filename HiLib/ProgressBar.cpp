#include "ProgressBar.h"
#include "D2DWWindow.h"
#include <boost/lexical_cast.hpp>

CProgressBar::CProgressBar(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl){}

void CProgressBar::OnPaint(const PaintEvent& e)
{
	//Draw background
	CRectF backRect(GetRectInWnd());
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), backRect);

	//Draw foreground
	CRectF foreRect(
		backRect.left,
		backRect.top,
		backRect.left + backRect.Width()*GetValue() / (GetMax() - GetMin()),
		backRect.bottom);
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetForeground(), foreRect);

	//Draw border
	GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(GetNormalBorder(), backRect);

	//Draw text
	std::wstring text = boost::lexical_cast<std::wstring>(GetValue()) + L" /  " + boost::lexical_cast<std::wstring>(GetMax() - GetMin());
	GetWndPtr()->GetDirectPtr()->DrawTextInRect(GetFormat(), text, backRect);
}
