#pragma once
#include "GridView.h"
#include "observable.h"

template<typename T>
class CBindGridView :public CGridView
{
private:
	std::observable_vector<T> m_itemsSource;
public:
	using CGridView::CGridView;

	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

	std::observable_vector<T>& GetItemsSource() { return m_itemsSource; }

	/******************/
	/* Window Message */
	/******************/
	//virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};