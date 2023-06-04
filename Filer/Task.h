#pragma once
#include "ReactiveProperty.h"
#include "Date.h"
#include "JsonSerializer.h"
#include "CheckBoxState.h"

struct Task
{
	auto operator<=>(const Task&) const = default;
	CheckBoxState State = CheckBoxState::False;
	std::wstring Name;
	std::wstring Memo;
	CDate Date;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("State", State);
		ar("Name", Name);
		ar("Memo", Memo);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		Task,
		State,
		Name,
		Memo,
		Date
	)
};

struct SubTask:public Task
{ };

struct MainTask:public Task
{
	//ReactiveVectorProperty<std::tuple<std::wstring>> Links;
	//ReactiveVectorProperty<std::tuple<SubTask>> SubTasks;

	template <class Archive>
	void serialize(Archive& ar)
	{
		Task::serialize(ar);
		//ar("SubTasks", SubTasks);
	}
};