#pragma once
#include "BindColumn.h"
#include "Sheet.h"
#include "Task.h"
#include "TaskMemoCell.h"

class CTaskMemoColumn :public CBindColumn<MainTask>
{
public:
	using CBindColumn<MainTask>::CBindColumn;

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CTaskMemoCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
	}
};
