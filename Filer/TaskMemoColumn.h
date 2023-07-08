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
		[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Memo->get_const(); },
		[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Memo->set(str); },
		arg<"celleditmode"_s>() = EditMode::ExcelLike){}
};

