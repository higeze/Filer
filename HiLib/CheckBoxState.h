#pragma once
#include <string>

enum class CheckBoxState
{
	None = 0,
	False = 1,
	Intermediate = 2,
	True = 3,
};

CheckBoxState Str2State(const std::wstring& str);
std::wstring State2Str(const CheckBoxState& state);

