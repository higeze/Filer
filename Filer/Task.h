#pragma once
#include "YearMonthDay.h"
#include "JsonSerializer.h"
#include "CheckBoxState.h"
#include "reactive_property.h"
#include "reactive_vector.h"
#include "any_tuple.h"

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
struct adl_vector_item<any_tuple>
{
	static any_tuple clone(const any_tuple& item) 
	{
		return any_tuple(item.get<MainTask>().Clone());
	}

	static void bind(any_tuple& src, any_tuple& dst)
	{
		src.get<MainTask>().State.binding(dst.get<MainTask>().State);
		src.get<MainTask>().Name.binding(dst.get<MainTask>().Name);
		src.get<MainTask>().Memo.binding(dst.get<MainTask>().Memo);
		src.get<MainTask>().YearMonthDay.binding(dst.get<MainTask>().YearMonthDay);
	}
};