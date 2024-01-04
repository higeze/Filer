#pragma once
#include "YearMonthDay.h"
#include "JsonSerializer.h"
#include "CheckBoxState.h"
#include "reactive_property.h"
#include "reactive_vector.h"

struct Task
{
public:
	reactive_property_ptr<CheckBoxState> State;
	reactive_property_ptr<std::wstring> Name;
	reactive_property_ptr<std::wstring> Memo;
	reactive_property_ptr<CYearMonthDay> YearMonthDay;

	Task():State(CheckBoxState::False),
		Name(), 
		Memo(),
		YearMonthDay(){}

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
		clone.State.set(*State);
		clone.Name.set(*Name);
		clone.Memo.set(*Memo);
		clone.YearMonthDay.set(*YearMonthDay);
		return clone;
	}
	//auto operator<=>(const MainTask&) const = default;
	bool operator==(const MainTask& value) const
	{
		return State == value.State && Name == value.Name && Memo == value.Memo && YearMonthDay == value.YearMonthDay;
	}
	//ReactiveVectorProperty<std::tuple<SubTask>> SubTasks;
};

template<>
struct adl_vector_item<std::tuple<MainTask>>
{
	static std::tuple<MainTask> clone(const std::tuple<MainTask>& item) 
	{
		return std::make_tuple(std::get<MainTask>(item).Clone());
	}

	static void bind(std::tuple<MainTask>& src, std::tuple<MainTask>& dst)
	{
		std::get<MainTask>(src).State.binding(std::get<MainTask>(dst).State);
		std::get<MainTask>(src).Name.binding(std::get<MainTask>(dst).Name);
		std::get<MainTask>(src).Memo.binding(std::get<MainTask>(dst).Memo);
		std::get<MainTask>(src).YearMonthDay.binding(std::get<MainTask>(dst).YearMonthDay);
	}
};