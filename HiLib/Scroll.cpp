#include "Scroll.h"
#include "GridView.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

namespace d2dw
{
	CScrollBase::CScrollBase(CGridView* pGrid) :CUIElement(), m_pGrid(pGrid){}


	void CScrollBase::SetScrollPos(const FLOAT pos)
	{
		FLOAT newPos = (std::max)((std::min)(pos, m_range.second - m_page), m_range.first);
		if (m_pos != newPos) {
			m_pos = newPos;
			OnPropertyChanged(L"pos");
		}
	}

	void CScrollBase::OnPaint(const PaintEvent& e)
	{
		if (!GetVisible())return;
		//Draw background
		e.Direct.FillSolidRectangle(m_backgroundFill, GetRect());
		//Draw thumb
		e.Direct.FillSolidRectangle(m_foregroundFill, GetThumbRect());
	}

	CRectF CVScroll::GetThumbRect()const
	{
		return
			CRectF(
				m_rect.left,
				(std::max)(m_rect.top + m_rect.Height() * GetScrollPos() / GetScrollDistance(), m_rect.top),
				m_rect.right,
				(std::min)(m_rect.top + m_rect.Height() * (GetScrollPos() + GetScrollPage()) / GetScrollDistance(), m_rect.bottom));
	}

	CRectF CHScroll::GetThumbRect()const
	{
		return
			CRectF(
				(std::max)(m_rect.left + m_rect.Width() * GetScrollPos() / GetScrollDistance(), m_rect.left),
				m_rect.top,
				(std::min)(m_rect.left + m_rect.Width() * (GetScrollPos() + GetScrollPage()) / GetScrollDistance(), m_rect.right),
				m_rect.bottom);
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

