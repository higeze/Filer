#pragma once

struct Task
{
	bool Done = false;
	std::wstring Name;
	std::wstring Memo;
};

struct SubTask:public Task
{ };

struct MainTask:public Task
{
	std::vector<std::wstring> Links;
	std::vector<SubTask> SubTasks;
};