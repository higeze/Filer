#include "CheckBoxState.h"

/********************/
/* Global functions */
/********************/

CheckBoxState Str2State(const std::wstring& str)
{
	return str.empty()? CheckBoxState::None : static_cast<CheckBoxState>(_wtoi(str.c_str()));
}

std::wstring State2Str(const CheckBoxState& state)
{
	return state == CheckBoxState::None ? L"" : std::to_wstring(static_cast<int>(state));
}
