#pragma once
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "Task.h"

class CTaskMemoColumn : public CBindTextColumn<MainTask>
{
public:
	template<typename... Args>
	CTaskMemoColumn(CSheet* pSheet = nullptr, Args... args)
		:CBindTextColumn(
		pSheet,
		L"Memo",
		[](const MainTask& value)->std::wstring {return *value.Memo; },
		[](MainTask& value, const std::wstring& str)->void { value.Memo.set(str); },
		arg<"celleditmode"_s>() = EditMode::ExcelLike){}
};

