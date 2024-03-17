#pragma once
#include "FilterCell.h"
#include "BindColumn.h"
#include "SortCell.h"
#include "GridView.h"
#include "named_arguments.h"
#include "TaskDueDateCell.h"
#include "Task.h"

class CTaskDueDateColumn: public CBindYearMonthDayColumn<MainTask>
{
public:
	template<typename... Args>
	CTaskDueDateColumn(CGridView* pSheet = nullptr, Args... args)
		:CBindYearMonthDayColumn<MainTask>(
		pSheet, 
		L"Due Date",
		[](MainTask& value)->reactive_property_ptr<CYearMonthDay>& { return value.YearMonthDay; },
		args...)
	{}

	virtual ~CTaskDueDateColumn(void) = default;

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CTaskDueDateCell>(
			m_pGrid, pRow, pColumn, m_pGrid->GetCellProperty());
	}
};

