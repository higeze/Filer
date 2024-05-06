#include "SplitContainer.h"
#include "D2DWWindow.h"

/******************/
/* SplitContainer */
/******************/
void CSplitContainer::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	m_one->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_two->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
}

void CSplitContainer::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	m_one->OnPaint(e);
	m_two->OnPaint(e);
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetSplitterRect());

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CSplitContainer::OnLButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	m_inDrag = true;
	m_ptBeginDrag = e.PointInWnd;
}

void CSplitContainer::OnLButtonEndDrag(const LButtonEndDragEvent& e)
{
	e.WndPtr->ReleaseCapturedControlPtr();
	m_inDrag = false;
	m_ptBeginDrag = CPointF();
}

/***************************/
/* CVerticalSplitContainer */
/***************************/

void CVerticalSplitContainer::OnMouseMove(const MouseMoveEvent& e)
{
	if (m_inDrag) {
		auto rc = GetRectInWnd();

		Value.set(std::clamp(*Value + (e.PointInWnd.x - m_ptBeginDrag.x), *Minimum, *Maximum));
		m_ptBeginDrag = e.PointInWnd;

		auto spParent = GetParentControlPtr();
		spParent->OnRect(RectEvent(spParent->GetWndPtr(), spParent->GetRectInWnd()));

		//During drag, SetCursor is not Fired.
		if (GetSplitterRect().PtInRect(e.PointInWnd)) {
			::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
			(*e.HandledPtr) = TRUE;
		}
	}
}

void CVerticalSplitContainer::OnSetCursor(const SetCursorEvent& e)
{
	if (GetSplitterRect().PtInRect(e.PointInWnd)) {
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		(*e.HandledPtr) = TRUE;
	}
}

CRectF CVerticalSplitContainer::GetSplitterRect() const
{
	CRectF rc = ArrangedRect();
	CRectF rcSplitter(*Value, rc.top, *Value + Width, rc.bottom);
	return rcSplitter;
}

void CVerticalSplitContainer::Measure(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_size.width = m_one->DesiredSize().width + Width + m_two->DesiredSize().width;
	m_size.height = (std::max)(m_one->DesiredSize().height, m_two->DesiredSize().height);
}

void CVerticalSplitContainer::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);

	Minimum.set(rc.left);
	Maximum.set(rc.right);
	
	if (*Value < 0) {//Initial
		Value.set((*Minimum + *Maximum) * 0.5f);
	} else {
		Value.set(std::clamp(*Value, *Minimum, *Maximum));
	}

	m_one->Arrange(CRectF(rc.left, rc.top, *Value, rc.bottom));
	m_two->Arrange(CRectF(*Value + Width, rc.top, rc.right, rc.bottom));
}

/*****************************/
/* CHorizontalSplitContainer */
/*****************************/

void CHorizontalSplitContainer::OnMouseMove(const MouseMoveEvent& e)
{
	if (m_inDrag) {
		auto rc = GetRectInWnd();

		Value.set(std::clamp(*Value + (e.PointInWnd.y - m_ptBeginDrag.y), *Minimum, *Maximum));
		m_ptBeginDrag = e.PointInWnd;

		auto spParent = GetParentControlPtr();
		spParent->OnRect(RectEvent(spParent->GetWndPtr(), spParent->GetRectInWnd()));

		//During drag, SetCursor is not Fired.
		if (GetSplitterRect().PtInRect(e.PointInWnd)) {
			::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
			(*e.HandledPtr) = TRUE;
		}
	}
}

void CHorizontalSplitContainer::OnSetCursor(const SetCursorEvent& e)
{
	if (GetSplitterRect().PtInRect(e.PointInWnd)) {
		::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
		(*e.HandledPtr) = TRUE;
	}
}

CRectF CHorizontalSplitContainer::GetSplitterRect() const
{
	CRectF rc = ArrangedRect();
	CRectF rcSplitter(rc.left, *Value, rc.right, *Value + Width);
	return rcSplitter;
}

void CHorizontalSplitContainer::Measure(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_size.width = (std::max)(m_one->DesiredSize().width, m_two->DesiredSize().width);
	m_size.height = m_one->DesiredSize().height + Width + m_two->DesiredSize().height;
}

void CHorizontalSplitContainer::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);

	Minimum.set(rc.top);
	Maximum.set(rc.bottom);
	
	if (*Value < 0) {//Initial
		Value.set((*Minimum + *Maximum) * 0.5f);
	} else {
		Value.set(std::clamp(*Value, *Minimum, *Maximum));
	}

	m_one->Arrange(CRectF(rc.left, rc.top, rc.right, *Value));
	m_two->Arrange(CRectF(rc.left, *Value + Width, rc.right, rc.bottom));
}
