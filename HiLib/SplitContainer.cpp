#include "SplitContainer.h"
#include "Splitter.h"
#include "D2DWWindow.h"

/******************/
/* SplitContainer */
/******************/
void CSplitContainer::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	m_one->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_two->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_splitter->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_splitter->Value.subscribe([this](auto value) { Arrange(ArrangedRect()); }, shared_from_this());
}

//void CSplitContainer::OnPaint(const PaintEvent& e)
//{
//	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);
//
//	m_one->OnPaint(e);
//	m_two->OnPaint(e);
//	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetSplitterRect());
//
//	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
//}

//void CSplitContainer::OnLButtonBeginDrag(const LButtonBeginDragEvent& e)
//{
//	if (GetSplitterRect().PtInRect(e.PointInWnd)) {
//		e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
//		m_inDrag = true;
//		m_ptBeginDrag = e.PointInWnd;
//		*e.HandledPtr = true;
//	}
//}
//
//void CSplitContainer::OnLButtonEndDrag(const LButtonEndDragEvent& e)
//{
//	if (m_inDrag) {
//		e.WndPtr->ReleaseCapturedControlPtr();
//		m_inDrag = false;
//		m_ptBeginDrag = CPointF();
//	}
//}

/***************************/
/* CVerticalSplitContainer */
/***************************/
CVerticalSplitContainer::CVerticalSplitContainer(CD2DWControl* pParentControl)
	:CSplitContainer(pParentControl)
{
	m_splitter = std::make_shared<CVerticalSplitter>(this);
}

//void CVerticalSplitContainer::OnMouseMove(const MouseMoveEvent& e)
//{
//	if (m_inDrag) {
//		auto rc = GetRectInWnd();
//
//		Value.set(std::clamp(*Value + (e.PointInWnd.x - m_ptBeginDrag.x), *Minimum, *Maximum));
//		m_ptBeginDrag = e.PointInWnd;
//
//		Arrange(rc);
//
//		//auto spParent = GetParentControlPtr();
//		//spParent->OnRect(RectEvent(spParent->GetWndPtr(), spParent->GetRectInWnd()));
//
//		//During drag, SetCursor is not Fired.
//		if (GetSplitterRect().PtInRect(e.PointInWnd)) {
//			::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
//			(*e.HandledPtr) = TRUE;
//		}
//	}
//}

//void CVerticalSplitContainer::OnSetCursor(const SetCursorEvent& e)
//{
//	if (GetSplitterRect().PtInRect(e.PointInWnd)) {
//		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
//		(*e.HandledPtr) = TRUE;
//	}
//}

//CRectF CVerticalSplitContainer::GetSplitterRect() const
//{
//	CRectF rc = ArrangedRect();
//	CRectF rcSplitter(*Value, rc.top, *Value + Width, rc.bottom);
//	return rcSplitter;
//}

void CVerticalSplitContainer::Measure(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_splitter->Measure(availableSize);
	m_size.width = m_one->DesiredSize().width +  m_two->DesiredSize().width +m_splitter->DesiredSize().width;
	m_size.height = (std::max)({m_one->DesiredSize().height, m_two->DesiredSize().height, m_splitter->DesiredSize().height});
}

void CVerticalSplitContainer::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	
	if (*m_splitter->Value < 0) {//Initial
		m_splitter->Value.set((rc.left, rc.right) * 0.5f);
	} 
	m_splitter->Minimum.set(rc.left);
	m_splitter->Maximum.set(rc.right);
	m_splitter->Arrange(CRectF(*m_splitter->Value, rc.top, *m_splitter->Value + m_splitter->DesiredSize().width, rc.bottom));
	m_one->Arrange(CRectF(rc.left, rc.top, m_splitter->ArrangedRect().left, rc.bottom));
	m_two->Arrange(CRectF(m_splitter->ArrangedRect().right, rc.top, rc.right, rc.bottom));
}

/*****************************/
/* CHorizontalSplitContainer */
/*****************************/
CHorizontalSplitContainer::CHorizontalSplitContainer(CD2DWControl* pParentControl)
	:CSplitContainer(pParentControl)
{
	m_splitter = std::make_shared<CHorizontalSplitter>(this);
}
//
//void CHorizontalSplitContainer::OnMouseMove(const MouseMoveEvent& e)
//{
//	if (m_inDrag) {
//		auto rc = GetRectInWnd();
//
//		Value.set(std::clamp(*Value + (e.PointInWnd.y - m_ptBeginDrag.y), *Minimum, *Maximum));
//		m_ptBeginDrag = e.PointInWnd;
//
//		Arrange(rc);
//
//		//auto spParent = GetParentControlPtr();
//		//spParent->OnRect(RectEvent(spParent->GetWndPtr(), spParent->GetRectInWnd()));
//
//		//During drag, SetCursor is not Fired.
//		if (GetSplitterRect().PtInRect(e.PointInWnd)) {
//			::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
//			(*e.HandledPtr) = TRUE;
//		}
//	}
//}
//
//void CHorizontalSplitContainer::OnSetCursor(const SetCursorEvent& e)
//{
//	if (GetSplitterRect().PtInRect(e.PointInWnd)) {
//		::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
//		(*e.HandledPtr) = TRUE;
//	}
//}

//CRectF CHorizontalSplitContainer::GetSplitterRect() const
//{
//	CRectF rc = ArrangedRect();
//	CRectF rcSplitter(rc.left, *Value, rc.right, *Value + Width);
//	return rcSplitter;
//}

void CHorizontalSplitContainer::Measure(const CSizeF& availableSize)
{
	m_one->Measure(availableSize);
	m_two->Measure(availableSize);
	m_splitter->Measure(availableSize);
	m_size.width = (std::max)({m_one->DesiredSize().width, m_two->DesiredSize().width, m_splitter->DesiredSize().width});
	m_size.height = m_one->DesiredSize().height + m_two->DesiredSize().height + m_splitter->DesiredSize().height;
}

void CHorizontalSplitContainer::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	
	if (*m_splitter->Value < 0) {//Initial
		m_splitter->Value.set((rc.top + rc.bottom) * 0.5f);
	}

	m_splitter->Minimum.set(rc.top);
	m_splitter->Maximum.set(rc.bottom);
	m_splitter->Arrange(CRectF(rc.left, *m_splitter->Value, rc.right, *m_splitter->Value + m_splitter->DesiredSize().height));
	m_one->Arrange(CRectF(rc.left, rc.top, rc.right, m_splitter->ArrangedRect().top));
	m_two->Arrange(CRectF(rc.left, m_splitter->ArrangedRect().bottom, rc.right, rc.bottom));
}
