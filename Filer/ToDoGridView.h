#pragma once
#include "BindGridView.h"
#include "Task.h"
#include "observable.h"

class CToDoGridView :public CBindGridView<MainTask>
{
private:
	observable<std::wstring> m_path;
public:
	template<typename... TArgs>
	CToDoGridView(std::shared_ptr<GridViewProperty>& spGridViewProp,
				  std::shared_ptr<observable_vector<std::tuple<MainTask>>> spItemsSource = nullptr,
				  TArgs... args)
		:CBindGridView<MainTask>(spGridViewProp, spItemsSource, args...){}

	virtual bool HasSheetCell() override { return true; }
	observable<std::wstring>& GetObsPath() { return m_path; }

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