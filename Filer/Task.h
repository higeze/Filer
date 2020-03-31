#pragma once
#include "observable.h"

struct Task
{
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
	observable_vector<std::wstring> Links;
	observable_vector<SubTask> SubTasks;

	template <class Archive>
	void serialize(Archive& ar)
	{
		Task::serialize(ar);
		ar("SubTasks", static_cast<std::vector<SubTask>&>(SubTasks));
	}
};