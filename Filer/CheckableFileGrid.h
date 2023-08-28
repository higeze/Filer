#pragma once
#include "FilerBindGridView.h"

class CCheckableFileGrid :public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
public:
	//Constructor
	CCheckableFileGrid(CD2DWControl* pParentControl, const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp);
	virtual ~CCheckableFileGrid() = default;

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnCellLButtonDblClk(const CellEventArgs& e)override;
	virtual void OpenFolder(const std::shared_ptr<CShellFolder>& spFile) override;

	void AddItem(const std::shared_ptr<CShellFile>& spFile);
};