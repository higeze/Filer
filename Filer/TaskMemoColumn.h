#pragma once
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "Task.h"

class CTaskMemoColumn : public CBindTextColumn
{
public:
	template<typename... Args>
	CTaskMemoColumn(CSheet* pSheet = nullptr, Args... args)
		:CBindTextColumn(
		pSheet,
		L"Memo",
		[](const any_tuple& tk)->std::wstring {return *tk.get<MainTask>().Memo; },
		[](any_tuple& tk, const std::wstring& str)->void { tk.get<MainTask>().Memo.set(str); },
		arg<"celleditmode"_s>() = EditMode::ExcelLike){}
};

