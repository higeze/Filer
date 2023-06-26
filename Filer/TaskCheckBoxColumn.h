#pragma once
#include "BindCheckBoxColumn.h"
#include "Task.h"

class CTaskCheckBoxColumn : public CBindCheckBoxColumn<MainTask>
{
public:
	CTaskCheckBoxColumn(CSheet* pSheet = nullptr)
		:CBindCheckBoxColumn<MainTask>(
		pSheet,
		L"State",
		[](const std::tuple<MainTask>& tk)->CheckBoxState {return std::get<MainTask>(tk).State.get(); },
		[](std::tuple<MainTask>& tk, const CheckBoxState& state)->void {std::get<MainTask>(tk).State.set(state); }){}
};

