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
struct adl_vector_item<MainTask>
{
	static MainTask clone(const MainTask& item) 
	{
		return item.Clone();
	}

	static void bind(MainTask& src, MainTask& dst)
	{
		src.State.binding(dst.State);
		src.Name.binding(dst.Name);
		src.Memo.binding(dst.Memo);
		src.YearMonthDay.binding(dst.YearMonthDay);
	}
};