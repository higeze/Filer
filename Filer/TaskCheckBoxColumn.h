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
		[](MainTask& value)->reactive_property_ptr<CheckBoxState>& {return value.State; }){}
};

