#include "Splitter.h"
#include "D2DWWindow.h"

void CHorizontalSplitter::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spSplitterProperty->BackgroundFill, GetRectInWnd());
	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();

}
void CHorizontalSplitter::OnLButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	m_inDrag = true;
	m_ptBeginDrag = e.PointInWnd;
}
void CHorizontalSplitter::OnLButtonEndDrag(const LButtonEndDragEvent& e)
{
	e.WndPtr->ReleaseCapturedControlPtr();
	m_inDrag = false;
	m_ptBeginDrag = CPointF();
}

void CHorizontalSplitter::OnMouseMove(const MouseMoveEvent& e)
{
	if (m_inDrag) {
		auto rc = GetRectInWnd();

		SplitterLeft.set(*SplitterLeft + (e.PointInWnd.x - m_ptBeginDrag.x));
		m_ptBeginDrag = e.PointInWnd;

		auto spParent = GetParentControlPtr();
		spParent->OnRect(RectEvent(spParent->GetWndPtr(), spParent->GetRectInWnd()));
	}
	::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	(*e.HandledPtr) = TRUE;

}
void CHorizontalSplitter::OnSetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	(*e.HandledPtr) = TRUE;
}
