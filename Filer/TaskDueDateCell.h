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
	CTaskDueDateCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, Args... args)
		:CBindYearMonthDayCell(pSheet, pRow, pColumn, args...)
	{}

	virtual ~CTaskDueDateCell() = default;

	void PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rc) override;

	//virtual bool CanSetStringOnEditing()const override{return false;}
};