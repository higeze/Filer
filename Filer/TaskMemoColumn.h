#pragma once
#include "BindTextColumn.h"
#include "TaskMemoCell.h"
#include "Task.h"

class CTaskMemoColumn : public CBindTextColumn<MainTask>
{
public:
	template<typename... Args>
	CTaskMemoColumn(CGridView* pSheet = nullptr, Args... args)
		:CBindTextColumn(
		pSheet,
		L"Memo",
		[](const MainTask& value)->std::wstring {return *value.Memo; },
		[](MainTask& value, const std::wstring& str)->void { value.Memo.set(str); },
		arg<"celleditmode"_s>() = EditMode::ExcelLike){}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CTaskMemoCell>(m_pGrid, pRow, pColumn, arg<"editmode"_s>() = m_cellEditMode);
	}
};

JSON_ENTRY_TYPE(CColumn, CTaskMemoColumn)


