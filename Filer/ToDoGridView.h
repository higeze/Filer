#pragma once
#include "BindGridView.h"
#include "Task.h"
#include "ReactiveProperty.h"
#include "TaskRow.h"
#include "ToDoDoc.h"

#include "reactive_property.h"
#include "reactive_command.h"

#include "RowIndexColumn.h"
#include "TaskCheckBoxColumn.h"
#include "TaskNameColumn.h"
#include "TaskMemoColumn.h"
#include "TaskDueDateColumn.h"



class CToDoGridView :public CBindGridView2<CTaskRow, CBindColumn<MainTask>, MainTask>
{
public:
	template<typename... TArgs>
	CToDoGridView(CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<GridViewProperty>& spGridViewProp = nullptr,
		const std::shared_ptr<ReactiveVectorProperty<std::tuple<MainTask>>> spItemsSource = nullptr,
		TArgs... args)
		:CBindGridView2<CTaskRow, CBindColumn<MainTask>, MainTask>(pParentControl, spGridViewProp, spItemsSource, args...),
		Path(make_reactive_property<std::wstring>()),
		OpenCommand(make_reactive_command<std::wstring>()),
		SaveCommand(make_reactive_command<std::wstring>())
	{
		Initialize();
	}
	void Initialize();


	virtual bool HasSheetCell() override { return true; }


	reactive_property_ptr<std::wstring> Path;
	reactive_command_ptr<std::wstring> OpenCommand;
	reactive_command_ptr<std::wstring> SaveCommand;

	void Open();
	void Open(const std::wstring& path);
	void Save();
	void Save(const std::wstring& path);
	/******************/
	/* Window Message */
	/******************/

	/****************/
	/* StateMachine */
	/****************/

	void Normal_ContextMenu(const ContextMenuEvent& e) override;
	void Normal_KeyDown(const KeyDownEvent& e) override;

	/********/
	/* JSON */
	/********/
	friend void to_json(json& j, const CToDoGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CRowIndexColumn);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskCheckBoxColumn);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskNameColumn);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskMemoColumn);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskDueDateColumn);
	
		j = json{
			{"Columns", o.m_allCols},
			{"RowFrozenCount", o.m_frozenRowCount},
			{"ColFrozenCount", o.m_frozenColumnCount}
		};
	}
	friend void from_json(const json& j, CToDoGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CRowIndexColumn);
		json_make_shared_map.insert_or_assign(typeid(CRowIndexColumn).name(), [&]() { return std::make_shared<CRowIndexColumn>(&o); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskCheckBoxColumn);
		json_make_shared_map.insert_or_assign(typeid(CTaskCheckBoxColumn).name(), [&]() { return std::make_shared<CTaskCheckBoxColumn>(&o); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskNameColumn);
		json_make_shared_map.insert_or_assign(typeid(CTaskNameColumn).name(), [&]() { return std::make_shared<CTaskNameColumn>(&o); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskMemoColumn);
		json_make_shared_map.insert_or_assign(typeid(CTaskMemoColumn).name(), [&]() { return std::make_shared<CTaskMemoColumn>(&o); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CTaskDueDateColumn);
		json_make_shared_map.insert_or_assign(typeid(CTaskDueDateColumn).name(), [&]() { return std::make_shared<CTaskDueDateColumn>(&o); });

		j.at("Columns").get_to(static_cast<std::vector<std::shared_ptr<CColumn>>&>(o.m_allCols));
		j.at("RowFrozenCount").get_to(o.m_frozenRowCount);
		j.at("ColFrozenCount").get_to(o.m_frozenColumnCount);

		for (auto& colPtr : o.m_allCols) {
			if (auto p = std::dynamic_pointer_cast<CTaskNameColumn>(colPtr)) {
				o.m_pNameColumn = p;
			} else if (auto p = std::dynamic_pointer_cast<CRowIndexColumn>(colPtr)) {
				o.m_pHeaderColumn = p;
			}
		}
	}

};