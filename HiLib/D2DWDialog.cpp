#include "D2DWDialog.h"
#include "DialogProperty.h"
#include "D2DWWindow.h"
#include "Direct2DWrite.h"
#include "DialogStateMachine.h"

CD2DWDialog::CD2DWDialog(CD2DWControl* pParentControl, const std::shared_ptr<DialogProperty>& spProp)
	:CD2DWControl(pParentControl),
	m_spProp(spProp),
	m_pDialogMachine(std::make_unique<CDialogStateMachine>(this))
{}

CD2DWDialog::~CD2DWDialog() = default;

void CD2DWDialog::OnCreate(const CreateEvt& e)
{ 
	CD2DWControl::OnCreate(e);
}
void CD2DWDialog::OnDestroy(const DestroyEvent& e)
{
	CD2DWControl::OnDestroy(e);
}
void CD2DWDialog::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CD2DWDialog::OnRect(const RectEvent& e)
{
	m_rect = e.Rect;
}

void CD2DWDialog::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_spProp->BackgroundFill, GetRectInWnd());

	PaintTitle();

	SendAllReverse(&CD2DWControl::OnPaint, e);

	PaintBorder();

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
}


void CD2DWDialog::OnLButtonBeginDrag(const LButtonBeginDragEvent& e){ m_pDialogMachine->process_event(e); }
void CD2DWDialog::OnLButtonEndDrag(const LButtonEndDragEvent& e){ m_pDialogMachine->process_event(e); }
void CD2DWDialog::OnMouseMove(const MouseMoveEvent& e){ m_pDialogMachine->process_event(e); }
void CD2DWDialog::OnMouseLeave(const MouseLeaveEvent& e){ m_pDialogMachine->process_event(e); }
void CD2DWDialog::OnSetCursor(const SetCursorEvent& e){ m_pDialogMachine->process_event(e); }


/****************/
/* StateMachine */
/****************/
bool CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_Moving(const LButtonBeginDragEvent& e)
{
	auto iter = std::find_if(m_childControls.crbegin(), m_childControls.crend(),
		[&](const std::shared_ptr<CD2DWControl>& x) {
			return *x->IsEnabled && x->GetRectInWnd().PtInRect(e.PointInWnd);
		});

	return iter == m_childControls.crend();
}
bool CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_LeftSizing(const LButtonBeginDragEvent& e)
{
	return IsPtInLeftSizingRect(e.PointInWnd);
}
bool CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_RightSizing(const LButtonBeginDragEvent& e)
{
	return IsPtInRightSizingRect(e.PointInWnd);
}
bool CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_TopSizing(const LButtonBeginDragEvent& e)
{
	return IsPtInTopSizingRect(e.PointInWnd);
}
bool CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_BottomSizing(const LButtonBeginDragEvent& e)
{
	return IsPtInBottomSizingRect(e.PointInWnd);
}

void CD2DWDialog::Normal_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	CD2DWControl::OnLButtonBeginDrag(e);
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::Normal_LButtonEndDrag(const LButtonEndDragEvent& e)
{
	CD2DWControl::OnLButtonEndDrag(e);
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::Normal_MouseMove(const MouseMoveEvent& e)
{
	CD2DWControl::OnMouseMove(e);
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::Normal_MouseLeave(const MouseLeaveEvent& e)
{
	CD2DWControl::OnMouseLeave(e);
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::Normal_SetCursor(const SetCursorEvent& e)
{
	if (IsPtInLeftSizingRect(e.PointInWnd)) {
		LeftSizing_SetCursor(e);
	} else if(IsPtInRightSizingRect(e.PointInWnd)){
		RightSizing_SetCursor(e);
	} else if (IsPtInTopSizingRect(e.PointInWnd)) {
		TopSizing_SetCursor(e);
	} else if (IsPtInBottomSizingRect(e.PointInWnd)) {
		BottomSizing_SetCursor(e);
	} else {
		CD2DWControl::OnSetCursor(e);
	}
	*e.HandledPtr = TRUE;
}
	
void CD2DWDialog::Moving_OnEntry(const LButtonBeginDragEvent& e)
{
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	m_startPoint = e.PointInWnd;
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::Moving_OnExit()
{
	GetWndPtr()->ReleaseCapturedControlPtr();
	m_startPoint.reset();
}
void CD2DWDialog::Moving_MouseMove(const MouseMoveEvent& e)
{
	auto rc = GetRectInWnd();
	rc.OffsetRect(CPointF(e.PointInWnd.x - m_startPoint.value().x, e.PointInWnd.y - m_startPoint.value().y));
	m_startPoint = e.PointInWnd;
	OnRect(RectEvent(GetWndPtr(), rc));
	*e.HandledPtr = TRUE;
}

void CD2DWDialog::Sizing_OnEntry(const LButtonBeginDragEvent& e)
{
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	m_startPoint = e.PointInWnd;
	*e.HandledPtr = TRUE;
}

void CD2DWDialog::Sizing_OnExit()
{
	GetWndPtr()->ReleaseCapturedControlPtr();
	m_startPoint.reset();
}

void CD2DWDialog::LeftSizing_MouseMove(const MouseMoveEvent& e)
{
	auto rc = GetRectInWnd();
	rc.left = e.PointInWnd.x;
	OnRect(RectEvent(GetWndPtr(), rc));
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::RightSizing_MouseMove(const MouseMoveEvent& e)
{
	auto rc = GetRectInWnd();
	rc.right = e.PointInWnd.x;
	OnRect(RectEvent(GetWndPtr(), rc));
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::TopSizing_MouseMove(const MouseMoveEvent& e)
{
	auto rc = GetRectInWnd();
	rc.top = e.PointInWnd.y;
	OnRect(RectEvent(GetWndPtr(), rc));
	*e.HandledPtr = TRUE;
}
void CD2DWDialog::BottomSizing_MouseMove(const MouseMoveEvent& e)
{
	auto rc = GetRectInWnd();
	rc.bottom = e.PointInWnd.y;
	OnRect(RectEvent(GetWndPtr(), rc));
	*e.HandledPtr = TRUE;
}

void CD2DWDialog::LeftSizing_SetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	*(e.HandledPtr) = TRUE;
}

void CD2DWDialog::RightSizing_SetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	*(e.HandledPtr) = TRUE;
}

void CD2DWDialog::TopSizing_SetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
	*(e.HandledPtr) = TRUE;
}

void CD2DWDialog::BottomSizing_SetCursor(const SetCursorEvent& e)
{
	::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
	*(e.HandledPtr) = TRUE;
}


void CD2DWDialog::Error_StdException(const std::exception& e)
{

}

CSizeF CD2DWDialog::GetTitleSize() const
{
	if (!Title->empty()) {
		m_titleSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(m_spProp->TitleFormat, *Title);
	}
	return m_titleSize;
}

CRectF CD2DWDialog::GetTitleRect()
{
	auto size = GetTitleSize();
	size.width += 2.f * kTitlePadding; size.height += 2.f * kTitlePadding;
	CRectF rc = size;
	rc.OffsetRect(GetRectInWnd().LeftTop());
	return rc;
}

void CD2DWDialog::PaintTitle()
{
	auto rc = GetTitleRect();
	rc.DeflateRect(kTitlePadding);
	GetWndPtr()->GetDirectPtr()->DrawTextLayout(m_spProp->TitleFormat, *Title, rc);
}

void CD2DWDialog::PaintBorder()
{
	auto rc = GetRectInWnd();
	rc.DeflateRect(m_spProp->Line.Width * 0.5f);
	GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(m_spProp->Line, rc);
}


/***********/
/* private */
/***********/
bool CD2DWDialog::IsPtInLeftSizingRect(const CPointF& pt)
{
	auto rc = GetRectInWnd();
	return CRectF(rc.left, rc.top, rc.left + kSizeWidth, rc.bottom).PtInRect(pt);
}
bool CD2DWDialog::IsPtInRightSizingRect(const CPointF& pt)
{
	auto rc = GetRectInWnd();
	return CRectF(rc.right - kSizeWidth, rc.top, rc.right, rc.bottom).PtInRect(pt);
}
bool CD2DWDialog::IsPtInTopSizingRect(const CPointF& pt)
{
	auto rc = GetRectInWnd();
	return CRectF(rc.left, rc.top, rc.right, rc.top + kSizeWidth).PtInRect(pt);
}

bool CD2DWDialog::IsPtInBottomSizingRect(const CPointF& pt)
{
	auto rc = GetRectInWnd();
	return CRectF(rc.left, rc.bottom - kSizeWidth, rc.right, rc.bottom).PtInRect(pt);
}


