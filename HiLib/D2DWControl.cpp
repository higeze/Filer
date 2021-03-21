#include "D2DWControl.h"
#include "D2DWWindow.h"

void CD2DWControl::SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl)
{
	if (m_pFocusedControl != spControl && spControl->GetIsFocusable().get()) {
		if (m_pFocusedControl) {
			m_pFocusedControl->OnKillFocus(KillFocusEvent(GetWndPtr(), 0, 0, nullptr));
		}
		m_pFocusedControl = spControl;
		m_pFocusedControl->OnSetFocus(SetFocusEvent(GetWndPtr(), 0, 0, nullptr));
	}
}

void CD2DWControl::AddChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl)
{
	if (std::find(m_childControls.cbegin(), m_childControls.cend(), pControl) == m_childControls.cend()){
		m_childControls.push_back(pControl);
		//m_pFocusedControl = pControl;
	} else {
		throw std::exception(FILE_LINE_FUNC);
	}
}

void CD2DWControl::EraseChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl)
{
	if (auto iter = std::find(m_childControls.cbegin(), m_childControls.cend(), pControl); iter != m_childControls.cend()) {
		if (m_pCapturedControl == pControl) { m_pCapturedControl = nullptr; }
		if (m_pFocusedControl == pControl) { m_pFocusedControl = nullptr; }
		if (m_pMouseControl == pControl) { m_pMouseControl = nullptr; }
		m_childControls.erase(iter);
	}
}


bool CD2DWControl::GetIsFocused()const
{
	//Parent Control is Window
	if (auto p = dynamic_cast<CD2DWWindow*>(GetParentControlPtr())) {
		auto a = p->GetFocusedControlPtr().get();
		auto b = const_cast<CD2DWControl*>(this);
		return p->GetFocusedControlPtr().get() == const_cast<CD2DWControl*>(this);
	//Parent Control is Control
	} else {
		auto a = GetParentControlPtr()->GetFocusedControlPtr().get();
		auto b = const_cast<CD2DWControl*>(this);
		return GetParentControlPtr()->GetIsFocused() && GetParentControlPtr()->GetFocusedControlPtr().get() == const_cast<CD2DWControl*>(this);
	}
}

/**************/
/* UI Message */
/**************/

void CD2DWControl::OnCreate(const CreateEvt& e)
{
	m_rect =  e.RectF;
	auto p = shared_from_this();
	GetParentControlPtr()->AddChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnClose(const CloseEvent& e)
{
	OnDestroy(e.WndPtr);
}

void CD2DWControl::OnDestroy(const DestroyEvent& e)
{
	GetParentControlPtr()->EraseChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnLButtonDown(const LButtonDownEvent& e) { SendPtInRect(&CD2DWControl::OnLButtonDown, e, true); }
void CD2DWControl::OnLButtonUp(const LButtonUpEvent& e) { SendPtInRect(&CD2DWControl::OnLButtonUp, e, true); }
void CD2DWControl::OnLButtonClk(const LButtonClkEvent& e) { SendPtInRect(&CD2DWControl::OnLButtonClk, e); }
void CD2DWControl::OnLButtonSnglClk(const LButtonSnglClkEvent& e) { SendPtInRect(&CD2DWControl::OnLButtonSnglClk, e); }
void CD2DWControl::OnLButtonDblClk(const LButtonDblClkEvent& e) { SendPtInRect(&CD2DWControl::OnLButtonDblClk, e); }

void CD2DWControl::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { SendCapturePtInRect(&CD2DWControl::OnLButtonBeginDrag, e); }
void CD2DWControl::OnLButtonEndDrag(const LButtonEndDragEvent& e) { SendCapturePtInRect(&CD2DWControl::OnLButtonEndDrag, e); }

void CD2DWControl::OnRButtonDown(const RButtonDownEvent& e) { SendPtInRect(&CD2DWControl::OnRButtonDown, e, true); }
void CD2DWControl::OnContextMenu(const ContextMenuEvent& e) { SendPtInRect(&CD2DWControl::OnContextMenu, e); }


void CD2DWControl::OnMouseMove(const MouseMoveEvent& e)
{
	if (GetCapturedControlPtr()) {
		GetCapturedControlPtr()->OnMouseMove(e);
	} else {

		auto iter = std::find_if(m_childControls.cbegin(), m_childControls.cend(),
			[&](const std::shared_ptr<CD2DWControl>& x) {
				return x->GetIsEnabled() && x->GetRectInWnd().PtInRect(e.PointInWnd);
			});

		if (iter == m_childControls.cend()) {//Mouse is NOT on child control, but on me.
			if (GetMouseControlPtr()) {
				GetMouseControlPtr()->OnMouseLeave(MouseLeaveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
			}
			SetMouseControlPtr(nullptr);
		} else if (GetMouseControlPtr() != *iter) {//Mouse is on child control and different with previous one
			if (GetMouseControlPtr()) {
				GetMouseControlPtr()->OnMouseLeave(MouseLeaveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
			}
			SetMouseControlPtr(*iter);
			GetMouseControlPtr()->OnMouseEnter(MouseEnterEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
		}

		if (GetMouseControlPtr()) { GetMouseControlPtr()->OnMouseMove(e); }
	}
}

void CD2DWControl::OnMouseLeave(const MouseLeaveEvent& e)
{
	if (GetMouseControlPtr()) {
		GetMouseControlPtr()->OnMouseLeave(MouseLeaveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
	}
	SetMouseControlPtr(nullptr);
}

void CD2DWControl::OnMouseWheel(const MouseWheelEvent& e) { SendPtInRect(&CD2DWControl::OnMouseWheel, e); }

void CD2DWControl::OnSetCursor(const SetCursorEvent& e)
{
	if (!GetCapturedControlPtr()) {
		SendPtInRect(&CD2DWControl::OnSetCursor, e);
	}
}

void CD2DWControl::OnSetFocus(const SetFocusEvent& e) { SendFocused(&CD2DWControl::OnSetFocus, e); }


void CD2DWControl::OnKeyDown(const KeyDownEvent& e) { SendFocused(&CD2DWControl::OnKeyDown, e); }
void CD2DWControl::OnSysKeyDown(const SysKeyDownEvent& e) { SendFocused(&CD2DWControl::OnSysKeyDown, e); }
void CD2DWControl::OnChar(const CharEvent& e) { SendFocused(&CD2DWControl::OnChar, e); }


void CD2DWControl::OnCommand(const CommandEvent& e)
{
	if (m_pFocusedControl) { m_pFocusedControl->OnCommand(e); }
	if (!(*e.HandledPtr)) {
		if (auto iter = m_commandMap.find(e.ID); iter != m_commandMap.end()) {
			iter->second(e);
		}
	}
	GetWndPtr()->InvalidateRect(NULL, FALSE);
}

