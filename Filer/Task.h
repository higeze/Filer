#pragma once

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
	std::vector<std::wstring> Links;
	std::vector<SubTask> SubTasks;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("SubTask", SubTasks);
	}
};