#pragma once
#include "BindCheckBoxColumn.h"
#include "TaskCheckBoxCell.h"
#include "Task.h"

class CTaskCheckBoxColumn : public CBindCheckBoxColumn<MainTask>
{
public:
	CTaskCheckBoxColumn(CGridView* pSheet = nullptr)
		:CBindCheckBoxColumn<MainTask>(
		pSheet,
		L"State",
		[](MainTask& value)->reactive_property_ptr<CheckBoxState>& {return value.State; }){}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CTaskCheckBoxCell>(m_pGrid, pRow, pColumn);
	}
};

JSON_ENTRY_TYPE(CColumn, CTaskCheckBoxColumn)

