#include "D2DWControl.h"
#include "D2DWWindow.h"

//void CD2DWControl::SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl)
//{
//	//if (m_pFocusedControl != spControl && *spControl->IsFocusable) {
//	//	if (m_pFocusedControl) {
//
//	//		m_pFocusedControl->OnKillFocus(KillFocusEvent(GetWndPtr(), 0, 0, nullptr));
//	//	}
//	//	m_pFocusedControl = spControl;
//	//	m_pFocusedControl->OnSetFocus(SetFocusEvent(GetWndPtr(), 0, 0, nullptr));
//	//}
//}

void CD2DWControl::AddChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl)
{
	if (std::find(m_childControls.cbegin(), m_childControls.cend(), pControl) == m_childControls.cend()){
		m_childControls.push_back(pControl);
		//m_pFocusedControl = pControl;
	} else {
		//throw std::exception(FILE_LINE_FUNC);
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

bool CD2DWControl::IsFocused()const
{
	return m_pParentControl->IsFocused() &&
		m_pParentControl->m_pFocusedControl.get() == const_cast<CD2DWControl*>(this);
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

/***************/
/* Control Msg */
/***************/
void CD2DWControl::OnCreate(const CreateEvt& e)
{
	m_pParentControl = e.ParentPtr;
	m_rect =  e.RectF;
	auto p = shared_from_this();
	GetParentControlPtr()->AddChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnClose(const CloseEvent& e)
{
	ProcessMessageToAllReverse(&CD2DWControl::OnClose, e);
	OnDestroy(DestroyEvent(e.WndPtr));
}

void CD2DWControl::OnDestroy(const DestroyEvent& e)
{
	//TODOHIGH
	//GetParentControlPtr()->EraseChildControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CD2DWControl::OnEnable(const EnableEvent& e) 
{
	IsEnabled.set(e.Enable);
	ProcessMessageToAll(&CD2DWControl::OnEnable, e);
}

void CD2DWControl::OnKeyDown(const KeyDownEvent& e)
{
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
					GetWndPtr()->SetFocusToControl((*focused_iter));
					*(e.HandledPtr) = TRUE;
				}
			}
		}
		break;
		default:
			break;
	}
}

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

CRectF CD2DWControl::CalcCenterRectF(const CSizeF& size) const
{
	auto rc = GetRectInWnd();
	return CRectF(
		rc.left + (rc.Width() - size.width) / 2,
		rc.top + (rc.Height() - size.height) / 2,
		rc.left + (rc.Width() + size.width) / 2,
		rc.top + (rc.Height() + size.height) / 2);
}

