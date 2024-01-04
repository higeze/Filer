#pragma once
#include "BindYearMonthDayColumn.h"
#include "FilterCell.h"
#include "BindColumn.h"
#include "SortCell.h"
#include "Sheet.h"
#include "named_arguments.h"
#include "TaskDueDateCell.h"
#include "Task.h"

class CTaskDueDateColumn: public CBindYearMonthDayColumn
{
public:
	template<typename... Args>
	CTaskDueDateColumn(CSheet* pSheet = nullptr, Args... args)
		:CBindYearMonthDayColumn(
		pSheet, 
		L"Due Date",
		[](any_tuple& tk)->reactive_property_ptr<CYearMonthDay>& { return tk.get<MainTask>().YearMonthDay; },
		args...)
	{}

	virtual ~CTaskDueDateColumn(void) = default;

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CTaskDueDateCell>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}
};

