#pragma once
#include "FilerBindGridView.h"
#include "observable.h"

class CCheckableFileGrid :public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
private:
	observable_vector<std::tuple<std::shared_ptr<CShellFile>>> m_itemsSource;
public:
	//Constructor
	CCheckableFileGrid(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp);
	virtual ~CCheckableFileGrid() {}

	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void OnCellLButtonDblClk(CellEventArgs& e)override;
	virtual void OpenFolder(std::shared_ptr<CShellFolder>& spFile) override;

	void AddItem(const std::shared_ptr<CShellFile>& spFile);
};