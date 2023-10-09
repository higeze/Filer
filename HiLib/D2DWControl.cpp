#include "D2DWControl.h"
#include "D2DWWindow.h"

void CD2DWControl::SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl)
{
	if (m_pFocusedControl != spControl && *spControl->IsFocusable) {
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
		if (GetWndPtr()->GetCapturedControlPtr() == pControl) { GetWndPtr()->ReleaseCapturedControlPtr(); }
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

//

void CD2DWControl::OnCreate(const CreateEvt& e)
{
	m_pParentControl = e.ParentPtr;
	m_rect =  e.RectF;
	auto p = shared_from_this();
	GetParentControlPtr()->AddChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnClose(const CloseEvent& e)
{
	SendCopyAll(&CD2DWControl::OnClose, e);
	OnDestroy(DestroyEvent(e.WndPtr));
}

void CD2DWControl::OnDestroy(const DestroyEvent& e)
{
	GetParentControlPtr()->EraseChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnEnable(const EnableEvent& e) 
{
	IsEnabled.set(e.Enable);
	SendAll(&CD2DWControl::OnEnable, e);
}
void CD2DWControl::OnLButtonDown(const LButtonDownEvent& e) { if (*IsEnabled) { SendPtInRectReverse(&CD2DWControl::OnLButtonDown, e, true); } }
void CD2DWControl::OnLButtonUp(const LButtonUpEvent& e) {  if (*IsEnabled){ SendPtInRectReverse(&CD2DWControl::OnLButtonUp, e, true); }}
void CD2DWControl::OnLButtonClk(const LButtonClkEvent& e) {  if (*IsEnabled) {SendPtInRectReverse(&CD2DWControl::OnLButtonClk, e); }}
void CD2DWControl::OnLButtonSnglClk(const LButtonSnglClkEvent& e) { if (*IsEnabled) { SendPtInRectReverse(&CD2DWControl::OnLButtonSnglClk, e); } }
void CD2DWControl::OnLButtonDblClk(const LButtonDblClkEvent& e) { if (*IsEnabled) { SendPtInRectReverse(&CD2DWControl::OnLButtonDblClk, e); } }

void CD2DWControl::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { if (*IsEnabled) { SendCapturePtInRectReverse(&CD2DWControl::OnLButtonBeginDrag, e); } }
void CD2DWControl::OnLButtonEndDrag(const LButtonEndDragEvent& e) { if (*IsEnabled){SendCapturePtInRectReverse(&CD2DWControl::OnLButtonEndDrag, e); }}

void CD2DWControl::OnRButtonDown(const RButtonDownEvent& e) { if (*IsEnabled){SendPtInRectReverse(&CD2DWControl::OnRButtonDown, e, true); }}
void CD2DWControl::OnContextMenu(const ContextMenuEvent& e) { if (*IsEnabled) { SendPtInRectReverse(&CD2DWControl::OnContextMenu, e); } }

void CD2DWControl::OnMButtonDown(const MButtonDownEvent& e) { if (*IsEnabled){SendPtInRectReverse(&CD2DWControl::OnMButtonDown, e, true); }}
void CD2DWControl::OnMButtonUp(const MButtonUpEvent& e) { if (*IsEnabled) { SendPtInRectReverse(&CD2DWControl::OnMButtonUp, e, true); } }


void CD2DWControl::OnMouseMove(const MouseMoveEvent& e)
{
	if (*IsEnabled) {
		if (GetWndPtr()->GetCapturedControlPtr()) {
			GetWndPtr()->GetCapturedControlPtr()->OnMouseMove(e);
		} else {

			auto iter = std::find_if(m_childControls.crbegin(), m_childControls.crend(),
				[&](const std::shared_ptr<CD2DWControl>& x) {
				return *x->IsEnabled && x->GetRectInWnd().PtInRect(e.PointInWnd);
			});

			if (iter == m_childControls.crend()) {//Mouse is NOT on child control, but on me.
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
}

void CD2DWControl::OnMouseLeave(const MouseLeaveEvent& e)
{
	if (*IsEnabled) {
		if (GetMouseControlPtr()) {
			GetMouseControlPtr()->OnMouseLeave(MouseLeaveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
		}
		SetMouseControlPtr(nullptr);
	}
}

void CD2DWControl::OnMouseWheel(const MouseWheelEvent& e) { if (*IsEnabled) { SendPtInRectReverse(&CD2DWControl::OnMouseWheel, e); } }

void CD2DWControl::OnSetCursor(const SetCursorEvent& e)
{
	if (*IsEnabled) {
		if (!GetWndPtr()->GetCapturedControlPtr()) {
			SendPtInRectReverse(&CD2DWControl::OnSetCursor, e);
		}
	}
}

void CD2DWControl::OnSetFocus(const SetFocusEvent& e) { if (*IsEnabled) { SendFocused(&CD2DWControl::OnSetFocus, e); } }
void CD2DWControl::OnKillFocus(const KillFocusEvent& e) { if (*IsEnabled){SendFocused(&CD2DWControl::OnKillFocus, e); }}

void CD2DWControl::OnWndSetFocus(const SetFocusEvent& e) { if (*IsEnabled){SendAllReverse(&CD2DWControl::OnWndSetFocus, e); }}
void CD2DWControl::OnWndKillFocus(const KillFocusEvent& e) { if (*IsEnabled){SendAllReverse(&CD2DWControl::OnWndKillFocus, e); }}

void CD2DWControl::OnKeyDown(const KeyDownEvent& e)
{
	if (*IsEnabled) {
		*(e.HandledPtr) = FALSE;
		SendFocused(&CD2DWControl::OnKeyDown, e);

		if (*(e.HandledPtr) == FALSE) {
			switch (e.Char) {
				case VK_TAB:
				{
					auto shift = ::GetAsyncKeyState(VK_SHIFT);
					auto focused_iter = std::find(m_childControls.crbegin(), m_childControls.crend(), GetFocusedControlPtr());
					if (focused_iter == m_childControls.crend()) {
						break;
					} else {
						focused_iter++;
					}
					for (auto iter = focused_iter; iter != m_childControls.crend(); ++iter) {
						if ((*iter)->GetIsTabStop()) {
							SetFocusedControlPtr((*focused_iter));
							*(e.HandledPtr) = TRUE;
						}
					}
				}
				break;
				default:
					break;
			}
		}
	}
}
void CD2DWControl::OnKeyUp(const KeyUpEvent& e) { if (*IsEnabled) { SendFocused(&CD2DWControl::OnKeyUp, e); } }
void CD2DWControl::OnKeyTraceDown(const KeyTraceDownEvent& e) { if (*IsEnabled) { SendFocused(&CD2DWControl::OnKeyTraceDown, e); } }
void CD2DWControl::OnKeyTraceUp(const KeyTraceUpEvent& e) { if (*IsEnabled) { SendFocused(&CD2DWControl::OnKeyTraceUp, e); } }
void CD2DWControl::OnSysKeyDown(const SysKeyDownEvent& e) { if (*IsEnabled) { SendFocused(&CD2DWControl::OnSysKeyDown, e); } }
void CD2DWControl::OnChar(const CharEvent& e)
{ 
	if (*IsEnabled) {
		*(e.HandledPtr) = FALSE;
		SendFocused(&CD2DWControl::OnChar, e);
	}
}
void CD2DWControl::OnImeStartComposition(const ImeStartCompositionEvent& e) { if (*IsEnabled) { SendFocused(&CD2DWControl::OnImeStartComposition, e); } }

void CD2DWControl::OnCommand(const CommandEvent& e)
{
	if (*IsEnabled) {
		if (auto iter = m_commandMap.find(e.ID); iter != m_commandMap.end()) {
			iter->second(e);
		}

		if (!(*e.HandledPtr) && m_pFocusedControl) {
			m_pFocusedControl->OnCommand(e);
		}
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}
}

CRectF CD2DWControl::CalcCenterRectF(const CSizeF& size)
{
	auto rc = GetRectInWnd();
	return CRectF(
		rc.left + (rc.Width() - size.width) / 2,
		rc.top + (rc.Height() - size.height) / 2,
		rc.left + (rc.Width() + size.width) / 2,
		rc.top + (rc.Height() + size.height) / 2);
}

