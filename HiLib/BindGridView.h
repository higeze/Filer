#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "observable.h"

template<typename T>
class CBindGridView :public CGridView
{
private:
	observable_vector<T> m_itemsSource;
public:
	CBindGridView(std::shared_ptr<GridViewProperty>& spGridViewProp)
		:CGridView(spGridViewProp)
	{
		m_itemsSource.VectorChanged.connect(
			[this](const NotifyVectorChangedEventArgs<T>& e)->void {
				switch (e.Action) {
				case NotifyVectorChangedAction::Add:
					PushRow(std::make_shared<CBindRow<T>>(this));
					break;
				case NotifyVectorChangedAction::Remove:
					EraseRow(m_allRows.back());
				default:
					break;
				}

			}
		);
	}

	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

	observable_vector<T>& GetItemsSource() { return m_itemsSource; }

	/******************/
	/* Window Message */
	/******************/
	//virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};