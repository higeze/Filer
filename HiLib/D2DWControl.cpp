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
void CD2DWControl::AddControlPtr(const std::shared_ptr<CD2DWControl>& pControl)
{
	if (std::find(m_pControls.cbegin(), m_pControls.cend(), pControl) == m_pControls.cend()){
		m_pControls.push_back(pControl);
		m_pFocusedControl = pControl;
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
	m_rect = e.RectF;
	GetParentControlPtr()->AddControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnLButtonDown(const LButtonDownEvent& e) 
{
	auto iter = std::find_if(m_pControls.begin(), m_pControls.end(),
		[&](const std::shared_ptr<CUIElement>& x) {return x->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

	SetFocusedControlPtr(iter != m_pControls.end() ? *iter : m_pControls.front());
	GetFocusedControlPtr()->OnLButtonDown(e);
}

void CD2DWControl::OnMouseMove(const MouseMoveEvent& e)
{
	auto iter = std::find_if(m_pControls.cbegin(), m_pControls.cend(),
		[&](const std::shared_ptr<CUIElement>& x) {return x->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

	if (iter == m_pControls.cend()) {//Mouse is NOT on child control, but on me.
		if (GetMouseControlPtr()) {
			GetMouseControlPtr()->OnMouseLeave(MouseLeaveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
		}
		SetMouseControlPtr(nullptr);
	} else if(GetMouseControlPtr() != *iter){//Mouse is on child control and different with previous one
		if (GetMouseControlPtr()) {
			GetMouseControlPtr()->OnMouseLeave(MouseLeaveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
		}
		SetMouseControlPtr(*iter);
		GetMouseControlPtr()->OnMouseEnter(MouseEnterEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
	}

	if (GetMouseControlPtr()) { GetMouseControlPtr()->OnMouseMove(e); }
}

void CD2DWControl::OnCommand(const CommandEvent& e)
{
	if (auto iter = m_commandMap.find(e.ID); iter != m_commandMap.end()) {
		iter->second(e);
	}
}

