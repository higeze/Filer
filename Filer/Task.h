#pragma once
#include "ReactiveProperty.h"
#include "YearMonthDay.h"
#include "JsonSerializer.h"
#include "CheckBoxState.h"
#include "reactive_property.h"

struct Task
{
public:
	reactive_property<CheckBoxState> State;
	reactive_property<std::wstring> Name;
	reactive_property<std::wstring> Memo;
	reactive_property<CYearMonthDay> YearMonthDay;

	Task():State(CheckBoxState::False), Name(), Memo(), YearMonthDay(){}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		Task,
		State,
		Name,
		Memo,
		YearMonthDay
	)
};

struct SubTask:public Task
{ };

struct MainTask:public Task
{
	MainTask Clone() const
	{
		MainTask clone;
		clone.State.set(State.get());
		clone.Name.set(Name.get());
		clone.Memo.set(Memo.get());
		clone.YearMonthDay.set(YearMonthDay.get().Clone());
		return clone;
	}
	//auto operator<=>(const MainTask&) const = default;
	bool operator==(const MainTask& value) const
	{
		return State == value.State && Name == value.Name && Memo == value.Memo && YearMonthDay == value.YearMonthDay;
	}

	//ReactiveVectorProperty<std::tuple<std::wstring>> Links;
	//ReactiveVectorProperty<std::tuple<SubTask>> SubTasks;

	template <class Archive>
	void serialize(Archive& ar)
	{
		Task::serialize(ar);
		//ar("SubTasks", SubTasks);
	}
};