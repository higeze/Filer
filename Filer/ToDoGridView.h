#pragma once
#include "BindGridView.h"
#include "Task.h"
#include "ReactiveProperty.h"
#include "TaskRow.h"

class CToDoGridView :public CBindGridView<CTaskRow, CBindColumn<MainTask>, MainTask>
{
private:
	ReactiveWStringProperty m_path;
public:
	template<typename... TArgs>
	CToDoGridView(CD2DWControl* pParentControl, 
				const std::shared_ptr<GridViewProperty>& spGridViewProp,
				const std::shared_ptr<ReactiveVectorProperty<std::tuple<MainTask>>> spItemsSource = nullptr,
				  TArgs... args)
		:CBindGridView<CTaskRow, CBindColumn<MainTask>, MainTask>(pParentControl, spGridViewProp, spItemsSource, args...)
	{
		Initialize();
	}

	void Initialize();

	virtual bool HasSheetCell() override { return true; }
	ReactiveWStringProperty& GetPath() { return m_path; }

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