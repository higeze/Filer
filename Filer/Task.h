#pragma once
#include "ReactiveProperty.h"
#include "Date.h"
#include "JsonSerializer.h"
#include "CheckBoxState.h"
#include "reactive_property.h"

struct Task
{
	auto operator<=>(const Task&) const = default;

	reactive_property<CheckBoxState> State = CheckBoxState::False;
	reactive_property<std::wstring> Name;
	reactive_property<std::wstring> Memo;
	reactive_property<CDate> Date;

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
	auto operator<=>(const MainTask&) const = default;
	//bool operator==(const MainTask& value) const
	//{
	//	return State == value.State && Name == value.Name && Memo == value.Memo && Date == value.Date;
	//}

	//ReactiveVectorProperty<std::tuple<std::wstring>> Links;
	//ReactiveVectorProperty<std::tuple<SubTask>> SubTasks;

	template <class Archive>
	void serialize(Archive& ar)
	{
		Task::serialize(ar);
		//ar("SubTasks", SubTasks);
	}
};