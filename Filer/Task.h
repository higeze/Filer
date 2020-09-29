#pragma once
#include "ReactiveProperty.h"

struct Task
{
	auto operator<=>(const Task&) const = default;
	bool Done = false;
	std::wstring Name;
	std::wstring Memo;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Done", Done);
		ar("Name", Name);
		ar("Memo", Memo);
	}
};

struct SubTask:public Task
{ };

struct MainTask:public Task
{
	ReactiveVectorProperty<std::tuple<std::wstring>> Links;
	ReactiveVectorProperty<std::tuple<SubTask>> SubTasks;

	template <class Archive>
	void serialize(Archive& ar)
	{
		Task::serialize(ar);
		ar("SubTasks", SubTasks);
	}
};