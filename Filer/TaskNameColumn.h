#pragma once
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "Task.h"

class CTaskNameColumn : public CBindTextColumn
{
private:
	using base = CBindTextColumn;
public:
	template<typename... Args>
	CTaskNameColumn(CSheet* pSheet = nullptr, Args... args)
		:CBindTextColumn(
		pSheet,
		L"Name",
		[](const any_tuple& tk)->std::wstring {return *tk.get<MainTask>().Name; },
		[](any_tuple& tk, const std::wstring& str)->void { tk.get<MainTask>().Name.set(str); },
		arg<"celleditmode"_s>() = EditMode::ExcelLike){}
};

