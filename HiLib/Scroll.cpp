#include "Scroll.h"
#include "GridView.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

namespace d2dw
{
	CScrollBase::CScrollBase(CGridView* pGrid, const std::shared_ptr<ScrollProperty>& spScrollProp)
		:CUIElement(), m_pGrid(pGrid), m_spScrollProp(spScrollProp){}


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
		e.Direct.FillSolidRectangle(m_spScrollProp->BackgroundFill, GetRect());
		//Draw thumb
		auto thumbFill = m_spScrollProp->ThumbNormalFill;
		if (GetState() == UIElementState::Dragged) {
			thumbFill = m_spScrollProp->ThumbScrollFill;
		} else if (CPoint pt = m_pGrid->GetCursorPosInClient();
			GetThumbRect().PtInRect(m_pGrid->GetDirect()->Pixels2Dips(pt))) {
			thumbFill = m_spScrollProp->ThumbHotFill;
		} else {
			thumbFill = m_spScrollProp->ThumbNormalFill;
		}
		e.Direct.FillSolidRectangle(thumbFill, GetThumbRect());
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

	void CVScroll::OnPropertyChanged(const wchar_t* name)
	{
		m_pGrid->PostUpdate(Updates::Row);
		m_pGrid->PostUpdate(Updates::Invalidate);
	}

	void CHScroll::OnPropertyChanged(const wchar_t* name)
	{
		m_pGrid->PostUpdate(Updates::Column);
		m_pGrid->PostUpdate(Updates::Invalidate);
	}


}

