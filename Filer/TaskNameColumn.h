#pragma once
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "Task.h"

class CTaskNameColumn : public CBindTextColumn<MainTask>
{
private:
	using base = CBindTextColumn<MainTask>;
public:
	template<typename... Args>
	CTaskNameColumn(CSheet* pSheet = nullptr, Args... args)
		:CBindTextColumn(
		pSheet,
		L"Name",
		[](const std::tuple<MainTask>& tk)->std::wstring {return *std::get<MainTask>(tk).Name; },
		[](std::tuple<MainTask>& tk, const std::wstring& str)->void { std::get<MainTask>(tk).Name.set(str); },
		arg<"celleditmode"_s>() = EditMode::ExcelLike){}
};

