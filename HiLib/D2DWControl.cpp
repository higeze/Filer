#include "D2DWControl.h"
#include "D2DWWindow.h"

void CD2DWControl::SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl)
{
	if (spControl->GetIsFocusable() && m_pFocusedControl != spControl) {
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
		m_pFocusedControl = pControl;
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
	} else {
		throw std::exception(FILE_LINE_FUNC);
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
		return GetParentControlPtr()->GetIsFocused();
	}
}

/**************/
/* UI Message */
/**************/

void CD2DWControl::OnCreate(const CreateEvt& e)
{
	m_rect =  e.RectF;
	GetParentControlPtr()->AddChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnClose(const CloseEvent& e)
{
	GetParentControlPtr()->EraseChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnLButtonDown(const LButtonDownEvent& e) { SetFocusSendPtInRect(&CD2DWControl::OnLButtonDown, e); }
void CD2DWControl::OnRButtonDown(const RButtonDownEvent& e) { SetFocusSendPtInRect(&CD2DWControl::OnRButtonDown, e); }


void CD2DWControl::OnMouseMove(const MouseMoveEvent& e)
{
	if (GetCapturedControlPtr()) {
		GetCapturedControlPtr()->OnMouseMove(e);
	} else {

		auto iter = std::find_if(m_childControls.cbegin(), m_childControls.cend(),
			[&](const std::shared_ptr<CUIElement>& x) { return x->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

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

