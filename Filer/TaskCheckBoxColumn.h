#pragma once
#include "BindCheckBoxColumn.h"
#include "Task.h"
#include "any_tuple.h"

class CTaskCheckBoxColumn : public CBindCheckBoxColumn
{
public:
	CTaskCheckBoxColumn(CSheet* pSheet = nullptr)
		:CBindCheckBoxColumn(
		pSheet,
		L"State",
		[](any_tuple& tk)->reactive_property_ptr<CheckBoxState>& {return tk.get<MainTask>().State; }){}
};

