#pragma once
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "TaskNameCell.h"
#include "Task.h"

class CTaskNameColumn : public CBindTextColumn<MainTask>
{
private:
	using base = CBindTextColumn<MainTask>;
public:
	template<typename... Args>
	CTaskNameColumn(CGridView* pSheet = nullptr, Args... args)
		:CBindTextColumn(
		pSheet,
		L"Name",
		[](const MainTask& value)->std::wstring {return *value.Name; },
		[](MainTask& value, const std::wstring& str)->void { value.Name.set(str); },
		arg<"celleditmode"_s>() = EditMode::ExcelLike){}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CTaskNameCell>(m_pGrid, pRow, pColumn, arg<"editmode"_s>() = m_cellEditMode);
	}
};

JSON_ENTRY_TYPE(CColumn, CTaskNameColumn)


