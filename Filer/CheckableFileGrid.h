#pragma once
#include "FilerBindGridView.h"
#include "ReactiveProperty.h"

class CCheckableFileGrid :public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
public:
	//Constructor
	CCheckableFileGrid(CD2DWControl* pParentControl, std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp);
	virtual ~CCheckableFileGrid() = default;

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnCellLButtonDblClk(CellEventArgs& e)override;
	virtual void OpenFolder(std::shared_ptr<CShellFolder>& spFile) override;

	void AddItem(const std::shared_ptr<CShellFile>& spFile);
};