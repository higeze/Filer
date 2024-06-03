#include "Splitter.h"
#include "D2DWWindow.h"

/*************/
/* CSplitter */
/*************/
CSplitter::CSplitter(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl), Minimum(-1.f), Maximum(-1.f), Value(FLT_MAX){}

void CSplitter::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	Minimum.subscribe([this](auto min) { Value.set((std::max)(min, *Value)); }, shared_from_this());
	Maximum.subscribe([this](auto max) { Value.set((std::min)(max, *Value)); }, shared_from_this());
}

void CSplitter::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());
	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CSplitter::OnLButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	m_ptBeginDrag.emplace(e.PointInWnd);
}

void CSplitter::OnLButtonEndDrag(const LButtonEndDragEvent& e)
{
	e.WndPtr->ReleaseCapturedControlPtr();
	m_ptBeginDrag.reset();
}

/***********************/
/* CHorizontalSplitter */
/***********************/
void CVerticalSplitter::OnMouseMove(const MouseMoveEvent& e)
{
	if (m_ptBeginDrag.has_value()) {
		Value.set(std::clamp(*Value + (e.PointInWnd.x - m_ptBeginDrag->x), *Minimum, *Maximum));
		m_ptBeginDrag.emplace(e.PointInWnd);

		OnSetCursor(SetCursorEvent(e.WndPtr, 0, e.HandledPtr));
		(*e.HandledPtr) = TRUE;
	}
}

void CVerticalSplitter::OnSetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	(*e.HandledPtr) = TRUE;
}

/*********************/
/* CVerticalSplitter */
/*********************/
void CHorizontalSplitter::OnMouseMove(const MouseMoveEvent& e)
{
	if (m_ptBeginDrag.has_value()) {
		Value.set(std::clamp(*Value + (e.PointInWnd.y - m_ptBeginDrag->y), *Minimum, *Maximum));
		m_ptBeginDrag.emplace(e.PointInWnd);

		OnSetCursor(SetCursorEvent(e.WndPtr, 0, e.HandledPtr));
		(*e.HandledPtr) = TRUE;
	}
}
void CHorizontalSplitter::OnSetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
	(*e.HandledPtr) = TRUE;
}