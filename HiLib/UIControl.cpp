#include "UIControl.h"
#include "TextboxWnd.h"

namespace d2dw
{
bool CUIControl::GetIsFocused()const
{
	return GetWndPtr()->GetFocusedControlPtr().get() == this;
}
}
