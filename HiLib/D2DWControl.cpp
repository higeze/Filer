#include "D2DWControl.h"
#include "D2DWWindow.h"


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

void CD2DWControl::OnCommand(const CommandEvent& e)
{
	if (auto iter = m_commandMap.find(e.ID); iter != m_commandMap.end()) {
		iter->second(e);
	}
}

