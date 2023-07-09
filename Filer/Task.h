#pragma once
#include "ReactiveProperty.h"
#include "YearMonthDay.h"
#include "JsonSerializer.h"
#include "CheckBoxState.h"
#include "reactive_property.h"

struct Task
{
public:
	reactive_property_ptr<CheckBoxState> State;
	reactive_property_ptr<std::wstring> Name;
	reactive_property_ptr<std::wstring> Memo;
	reactive_property_ptr<CYearMonthDay> YearMonthDay;

	Task():State(make_reactive_property<CheckBoxState>(CheckBoxState::False)),
		Name(make_reactive_property<std::wstring>()), 
		Memo(make_reactive_property<std::wstring>()),
		YearMonthDay(make_reactive_property<CYearMonthDay>()){}

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
		clone.State->set(State->get_const());
		clone.Name->set(Name->get_const());
		clone.Memo->set(Memo->get_const());
		clone.YearMonthDay->set(YearMonthDay->get_const());
		return clone;
	}
	//auto operator<=>(const MainTask&) const = default;
	bool operator==(const MainTask& value) const
	{
		return State == value.State && Name == value.Name && Memo == value.Memo && YearMonthDay == value.YearMonthDay;
	}
	//ReactiveVectorProperty<std::tuple<SubTask>> SubTasks;
};