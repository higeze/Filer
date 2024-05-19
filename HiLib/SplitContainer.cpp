#include "SplitContainer.h"
#include "Splitter.h"
#include "D2DWWindow.h"

/**************/
/* CDockPanel */
/**************/
void CDockPanel::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);

	CreateEvt evt = CreateEvt(GetWndPtr(), this, CRectF());
	for (const auto& p : m_ctrl_split_map) {
		p.first->OnCreate(evt);
	}
	for (const auto& p : m_ctrl_split_map) {
		if (p.second) {
			p.second->OnCreate(evt);
		}
	}
}

void CDockPanel::Measure(const CSizeF& availableSize)
{
	m_size = CSizeF();
	for (const auto& child : m_childControls) {
		child->Measure(availableSize);
		switch (*child->Dock) {
			case DockEnum::Left:
			case DockEnum::Right:
			case DockEnum::LeftFix:
			case DockEnum::RightFix:
			case DockEnum::Vertical:
			{
				m_size.width += child->DesiredSize().width;
				break;
			}
			case DockEnum::Top:
			case DockEnum::Bottom:
			case DockEnum::TopFix:
			case DockEnum::BottomFix:
			case DockEnum::Horizontal:
			{
				m_size.height += child->DesiredSize().height;
				break;
			}
			case DockEnum::Fill:
			{
				m_size.width += child->DesiredSize().width;
				m_size.height += child->DesiredSize().height;
				break;
			}
			default:
				THROW_FILE_LINE_FUNC;
				break;
		}
	}
}

void CDockPanel::Arrange(const CRectF& rc)
{
	//CD2DWControl::Arrange(rc);
	//
	//if (*m_splitter->Value < 0) {//Initial
	//	m_splitter->Value.set((rc.left, rc.right) * 0.5f);
	//} 
	//m_splitter->Minimum.set(rc.left);sp
	//m_splitter->Maximum.set(rc.right);
	//m_splitter->Arrange(CRectF(*m_splitter->Value, rc.top, *m_splitter->Value + m_splitter->DesiredSize().width, rc.bottom));
	//m_one->Arrange(CRectF(rc.left, rc.top, m_splitter->ArrangedRect().left, rc.bottom));
	//m_two->Arrange(CRectF(m_splitter->ArrangedRect().right, rc.top, rc.right, rc.bottom));

	CD2DWControl::Arrange(rc);

	CRectF remain = rc;
	FLOAT width = 5;
	for (const auto& child : m_childControls) {
		switch (*child->Dock) {
			case DockEnum::TopFix:
			{
				child->Arrange(CRectF(remain.left, remain.top, remain.right, remain.top + child->DesiredSize().height));
				remain.top = child->ArrangedRect().bottom;
				break;
			}
			case DockEnum::BottomFix:
			{
				child->Arrange(CRectF(remain.left, remain.bottom - child->DesiredSize().height, remain.right, remain.bottom));
				remain.bottom = child->ArrangedRect().top;
				break;
			}
			case DockEnum::Left:
			{
				//TODOTODO
				break;
			}
			case DockEnum::Top:
			{
				//TODOTODO
				break;
			}
			case DockEnum::Right:
			{
				auto split = m_ctrl_split_map[child];
				split->Value.block();
				auto minimum = remain.left;
				auto maximum = remain.right - split->DesiredSize().width;
				if (*split->Value < 0) {//Initial
					split->Value.set(std::clamp(maximum - child->DesiredSize().width, minimum, maximum));
				} else {
					if (maximum - minimum != *split->Maximum - *split->Minimum) {
						split->Value.set(std::clamp((*split->Value + (maximum - *split->Maximum)), minimum, maximum));
					} 
				}
				split->Minimum.set(minimum);
				split->Maximum.set(maximum);
				split->Arrange(CRectF(*split->Value, remain.top, *split->Value + split->DesiredSize().width, remain.bottom));
				child->Arrange(CRectF(split->ArrangedRect().right, remain.top, remain.right, remain.bottom));
				remain.right = split->ArrangedRect().left;
				split->Value.unblock();
				break;
			}
			case DockEnum::Bottom:
			{
				auto split = m_ctrl_split_map[child];
				split->Value.block();
				auto minimum = remain.top;
				auto maximum = remain.bottom - split->DesiredSize().height;
				if (*split->Value < 0) {//Initial
					split->Value.set(std::clamp(maximum - child->DesiredSize().height, minimum, maximum));
				} else {
					if (maximum - minimum != *split->Maximum - *split->Minimum) {
						split->Value.set(std::clamp((*split->Value + (maximum - *split->Maximum)), minimum, maximum));
					} 
				}
				split->Minimum.set(minimum);
				split->Maximum.set(maximum);
				split->Arrange(CRectF(remain.left, *split->Value, remain.right, *split->Value + split->DesiredSize().height));
				child->Arrange(CRectF(remain.left, split->ArrangedRect().bottom, remain.right, remain.bottom));
				remain.bottom = split->ArrangedRect().top;
				split->Value.unblock();
				break;
			}
			case DockEnum::Fill:
				child->Arrange(remain);
				break;
			case DockEnum::Vertical:
			case DockEnum::Horizontal:
				break;
			default:
				THROW_FILE_LINE_FUNC;
				break;
		}
	}
	
	//if (*m_splitter->Value < 0) {//Initial
	//	m_splitter->Value.set((rc.left, rc.right) * 0.5f);
	//} 
	//m_splitter->Minimum.set(rc.left);
	//m_splitter->Maximum.set(rc.right);
	//m_splitter->Arrange(CRectF(*m_splitter->Value, rc.top, *m_splitter->Value + m_splitter->DesiredSize().width, rc.bottom));
	//m_one->Arrange(CRectF(rc.left, rc.top, m_splitter->ArrangedRect().left, rc.bottom));
	//m_two->Arrange(CRectF(m_splitter->ArrangedRect().right, rc.top, rc.right, rc.bottom));
}


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
