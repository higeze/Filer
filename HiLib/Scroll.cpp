#include "Scroll.h"
#include "D2DWWindow.h"

CScrollBase::CScrollBase(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl),
	Position(0.f),
	m_visibility(Visibility::Auto){}

bool CScrollBase::GetIsVisible()const
{
	switch (GetVisibility()) {
		case Visibility::Disabled:
			return false;
		case Visibility::Auto:
			return GetScrollDistance() > GetScrollPage();
		case Visibility::Hidden:
			return false;
		case Visibility::Visible:
			return true;
		default:
			return false;
	}
}

void CScrollBase::SetScrollPos(const FLOAT pos)
{
	FLOAT newPos = std::clamp(pos, m_range.first, (std::max)(m_range.second - m_page, m_range.first));
	if (*Position != newPos) {
		Position.set(newPos);
		ScrollChanged();
	}
}

void CScrollBase::SetScrollPage(const FLOAT page) 
{ 
	if (m_page != page) {
		m_page = page;
		//SetScrollPos(GetScrollPos());//Need clamp
		ScrollChanged();
	}
}
	
void CScrollBase::SetScrollRange(const FLOAT min, FLOAT max)
{
	if (m_range.first != min || m_range.second != max) {
		m_range.first = min; m_range.second = max;
		SetScrollPos(GetScrollPos());//Need clamp
		ScrollChanged();
	}
}

void CScrollBase::SetScrollRangePage(const FLOAT min, const FLOAT max, const FLOAT page)
{
	bool isRangeChange = m_range.first != min || m_range.second != max;
	if (isRangeChange) {
		m_range.first = min; m_range.second = max;
		Position.set(std::clamp(GetScrollPos(), m_range.first, (std::max)(m_range.second - m_page, m_range.first)));
	}
	bool isPageChange = m_page != page;
	if (isPageChange) {
		m_page = page;
	}
	if (isRangeChange || isPageChange) {
		ScrollChanged();
	}
}

void CScrollBase::OnSetCursor(const SetCursorEvent& e)
{
	auto pt = e.WndPtr->GetCursorPosInWnd();
	if (GetRectInWnd().PtInRect(pt)) {
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		*(e.HandledPtr) = TRUE;
	}
}

void CScrollBase::OnPaint(const PaintEvent& e)
{
	if (!GetIsVisible())return;
	PaintBackground(e);
	PaintThumb(e);
	PaintForeground(e);
}

void CScrollBase::PaintBackground(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());
}


void CScrollBase::PaintThumb(const PaintEvent& e)
{
	auto thumbFill = GetThumbNormalFill();
	if (GetState() == UIElementState::Dragged) {
		thumbFill = GetThumbScrollFill();
	} else if (CPoint pt = GetWndPtr()->GetCursorPosInClient();
		GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(pt))) {
		thumbFill = GetThumbHotFill();
	} else {
		thumbFill = GetThumbNormalFill();
	}
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(thumbFill, GetThumbRect());
}

CRectF CScrollBase::GetThumbRangeRect()const
{
	auto thumbRangeRect = GetRectInWnd();
	thumbRangeRect.DeflateRect(GetThumbMargin());
	return thumbRangeRect;
}

void CScrollBase::Clear()
{
	m_visibility = Visibility::Auto;
	m_page = 0.f;
	m_range = std::make_pair(0.f, 0.f);
	//m_pos = 0.f;
	m_startDrag = 0.f;
}

CRectF CVScroll::GetThumbRect()const
{
	auto thumbRangeRect = GetThumbRangeRect();
	return
		CRectF(
			thumbRangeRect.left,
			(std::max)(thumbRangeRect.top + thumbRangeRect.Height() * GetScrollPos() / GetScrollDistance(), thumbRangeRect.top),
			thumbRangeRect.right,
			(std::min)(thumbRangeRect.top + thumbRangeRect.Height() * (GetScrollPos() + GetScrollPage()) / GetScrollDistance(), thumbRangeRect.bottom));
}

CRectF CHScroll::GetThumbRect()const
{
	auto thumbRangeRect = GetThumbRangeRect();
	return
		CRectF(
			(std::max)(thumbRangeRect.left + thumbRangeRect.Width() * GetScrollPos() / GetScrollDistance(), thumbRangeRect.left),
			thumbRangeRect.top,
			(std::min)(thumbRangeRect.left + thumbRangeRect.Width() * (GetScrollPos() + GetScrollPage()) / GetScrollDistance(), thumbRangeRect.right),
			thumbRangeRect.bottom);
}




