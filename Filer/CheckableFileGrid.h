#pragma once

#include "FilerGridViewBase.h"

class CDirectoryWatcher;

class CCheckableFileGrid :public CFilerGridViewBase
{
private:
	//For Context Menu
	//HMENU m_hNewMenu;

	//CComPtr<IContextMenu2> m_pcmNew2;
	//CComPtr<IContextMenu3> m_pcmNew3;

	//CComPtr<IContextMenu2> m_pcm2;
	//CComPtr<IContextMenu3> m_pcm3;
public:
	//Constructor
	CCheckableFileGrid(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp);
	virtual ~CCheckableFileGrid() {}

	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

//	virtual LRESULT OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

//	virtual void OnKeyDown(const KeyDownEvent& e)override;
//	virtual void OnContextMenu(const ContextMenuEvent& e) override;
	virtual void OnCellLButtonDblClk(CellEventArgs& e)override;
	void Open(std::shared_ptr<CShellFile>& spFile);
	void OpenFolder(std::shared_ptr<CShellFolder>& spFile);



//	virtual bool CutToClipboard() override;
//	virtual bool CopyToClipboard() override;
//	virtual bool PasteFromClipboard() override;
//	virtual bool Delete() override;
	void AddItem(const std::shared_ptr<CShellFile>& spFile);

protected:
	void InsertDefaultRowColumn() override;
};