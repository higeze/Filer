#pragma once
#include "BindGridView.h"
#include "Task.h"
#include "TaskRow.h"
#include "ToDoDoc.h"

#include "reactive_property.h"
#include "reactive_command.h"

#include "RowIndexColumn.h"
#include "TaskCheckBoxColumn.h"
#include "TaskNameColumn.h"
#include "TaskMemoColumn.h"
#include "TaskDueDateColumn.h"



class CToDoGridView :public CBindGridView<MainTask, CTaskRow>
{
public:
	template<typename... TArgs>
	CToDoGridView(CD2DWControl* pParentControl = nullptr,
		TArgs... args)
		:CBindGridView<MainTask, CTaskRow>(pParentControl, args...),
		Path()
	{
		m_gridViewMode = GridViewMode::ExcelLike;
		Initialize();
	}
	void Initialize();

	//virtual bool HasSheetCell() override { return true; }

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
		json_safe_to(j, "Columns", o.m_allCols);
		json_safe_to(j, "RowFrozenCount", o.m_frozenRowCount);
		json_safe_to(j, "ColFrozenCount", o.m_frozenColumnCount);
	}
	friend void from_json(const json& j, CToDoGridView& o)
	{
		json_safe_from(j, "Columns", static_cast<std::vector<std::shared_ptr<CColumn>>&>(o.m_allCols));
		json_safe_from(j, "RowFrozenCount", o.m_frozenRowCount);
		json_safe_from(j, "ColFrozenCount", o.m_frozenColumnCount);

		for (auto& col : o.m_allCols) {
			col->SetSheetPtr(&o);
		}
	}

};