#include "Scroll.h"
#include "ScrollProperty.h"
#include "GridView.h"
#include "TextboxWnd.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

namespace d2dw
{
	CScrollBase::CScrollBase(CWindow* pWnd, const std::shared_ptr<ScrollProperty>& spScrollProp, std::function<void(const wchar_t*)> onPropertyChanged)
		:CUIControl(pWnd), m_spScrollProp(spScrollProp), m_onPropertyChanged(onPropertyChanged){}

	FLOAT CScrollBase::GetScrollBandWidth()const { return m_spScrollProp->BandWidth; }
	FLOAT CScrollBase::GetScrollDelta()const { return m_spScrollProp->DeltaScroll; }


	void CScrollBase::SetScrollPos(const FLOAT pos)
	{
		FLOAT newPos = std::clamp(pos, m_range.first, (std::max)(m_range.second - m_page, m_range.first));
		if (m_pos != newPos) {
			m_pos = newPos;
			OnPropertyChanged(L"pos");
		}
	}

	void CScrollBase::SetScrollPage(const FLOAT page) 
	{ 
		if (m_page != page) {
			m_page = page;
			SetScrollPos(GetScrollPos());//Need clamp
		}
	}
	
	void CScrollBase::SetScrollRange(const FLOAT min, FLOAT max)
	{
		if (m_range.first != min || m_range.second != max) {
			m_range.first = min; m_range.second = max;
			SetScrollPos(GetScrollPos());//Need clamp
		}
	}


	void CScrollBase::OnPaint(const PaintEvent& e)
	{
		if (!GetVisible())return;
		//Draw background
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spScrollProp->BackgroundFill, GetRectInWnd());
		//Draw thumb
		auto thumbFill = m_spScrollProp->ThumbNormalFill;
		if (GetState() == UIElementState::Dragged) {
			thumbFill = m_spScrollProp->ThumbScrollFill;
		} else if (CPoint pt = GetWndPtr()->GetCursorPosInClient();
			GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(pt))) {
			thumbFill = m_spScrollProp->ThumbHotFill;
		} else {
			thumbFill = m_spScrollProp->ThumbNormalFill;
		}
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(thumbFill, GetThumbRect());
	}

	CRectF CVScroll::GetThumbRect()const
	{
		return
			CRectF(
				m_rect.left + m_spScrollProp->ThumbMargin.left,
				(std::max)(m_rect.top + m_rect.Height() * GetScrollPos() / GetScrollDistance(), m_rect.top + m_spScrollProp->ThumbMargin.top),
				m_rect.right - m_spScrollProp->ThumbMargin.right,
				(std::min)(m_rect.top + m_rect.Height() * (GetScrollPos() + GetScrollPage()) / GetScrollDistance(), m_rect.bottom - m_spScrollProp->ThumbMargin.bottom));
	}

	CRectF CHScroll::GetThumbRect()const
	{
		return
			CRectF(
				(std::max)(m_rect.left + m_rect.Width() * GetScrollPos() / GetScrollDistance(), m_rect.left + m_spScrollProp->ThumbMargin.left),
				m_rect.top + m_spScrollProp->ThumbMargin.top,
				(std::min)(m_rect.left + m_rect.Width() * (GetScrollPos() + GetScrollPage()) / GetScrollDistance(), m_rect.right - m_spScrollProp->ThumbMargin.right),
				m_rect.bottom - m_spScrollProp->ThumbMargin.bottom);
	}

}

