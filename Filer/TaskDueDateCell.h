#pragma once
#include "BindYearMonthDayCell.h"
#include "D2DWWindow.h"
#include "ResourceIDFactory.h"
#include "YearMonthDay.h"
#include "Task.h"

class CTaskDueDateCell :public CBindYearMonthDayCell<MainTask>
{
public:
	template<typename... Args>
	CTaskDueDateCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CBindYearMonthDayCell(pSheet, pRow, pColumn, spProperty, args...)
	{}

	virtual ~CTaskDueDateCell() = default;

	//virtual bool CanSetStringOnEditing()const override{return false;}
};