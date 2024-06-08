#pragma once
#include "FilerBindGridView.h"

class CCheckableFileGrid :public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
public:
	//Constructor
	CCheckableFileGrid(CD2DWControl* pParentControl);
	virtual ~CCheckableFileGrid() = default;

	//virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnCellLButtonDblClk(const CellEventArgs& e)override;
	virtual void OpenFolder(const std::shared_ptr<CShellFolder>& spFile, bool isReload) override;

	void AddItem(const std::shared_ptr<CShellFile>& spFile);
};