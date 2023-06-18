#pragma once
#include "BindGridView.h"
#include "Task.h"
#include "ReactiveProperty.h"
#include "TaskRow.h"
#include "ToDoDoc.h"

#include "reactive_property.h"
#include "reactive_command.h"

class CToDoGridView :public CBindGridView2<CTaskRow, CBindColumn<MainTask>, MainTask>
{
public:
	template<typename... TArgs>
	CToDoGridView(CD2DWControl* pParentControl, 
				const std::shared_ptr<GridViewProperty>& spGridViewProp,
				const std::shared_ptr<ReactiveVectorProperty<std::tuple<MainTask>>> spItemsSource = nullptr,
				  TArgs... args)
		:CBindGridView2<CTaskRow, CBindColumn<MainTask>, MainTask>(pParentControl, spGridViewProp, spItemsSource, args...)
	{
		Initialize();
	}

	void Initialize();


	virtual bool HasSheetCell() override { return true; }


	reactive_property<std::wstring> Path;
	reactive_command<std::wstring> OpenCommand;
	reactive_command<std::wstring> SaveCommand;

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
};