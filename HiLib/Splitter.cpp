#include "Splitter.h"
#include "D2DWWindow.h"

/*************/
/* CSplitter */
/*************/

void CSplitter::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());
	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();

}
void CSplitter::OnLButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	m_inDrag = true;
	m_ptBeginDrag = e.PointInWnd;
}
void CSplitter::OnLButtonEndDrag(const LButtonEndDragEvent& e)
{
	e.WndPtr->ReleaseCapturedControlPtr();
	m_inDrag = false;
	m_ptBeginDrag = CPointF();
}

/***********************/
/* CHorizontalSplitter */
/***********************/

void CHorizontalSplitter::OnMouseMove(const MouseMoveEvent& e)
{
	if (m_inDrag) {
		auto rc = GetRectInWnd();

		Value.set(std::clamp(*Value + (e.PointInWnd.x - m_ptBeginDrag.x), *Minimum, *Maximum));
		::OutputDebugStringA(std::format("{},{},{}\r\n", *Value, *Minimum, *Maximum).c_str());
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

/*********************/
/* CVerticalSplitter */
/*********************/

void CVerticalSplitter::OnMouseMove(const MouseMoveEvent& e)
{
	if (m_inDrag) {
		auto rc = GetRectInWnd();

		Value.set(std::clamp(*Value + (e.PointInWnd.y - m_ptBeginDrag.y), *Minimum, *Maximum));
		m_ptBeginDrag = e.PointInWnd;

		auto spParent = GetParentControlPtr();
		spParent->OnRect(RectEvent(spParent->GetWndPtr(), spParent->GetRectInWnd()));
	}
	::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
	(*e.HandledPtr) = TRUE;

}
void CVerticalSplitter::OnSetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
	(*e.HandledPtr) = TRUE;
}