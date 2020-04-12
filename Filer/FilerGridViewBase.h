#pragma once

#include "GridView.h"
#include "IDL.h"
#include "Resource.h"

#include "FilerGridViewProperty.h"

#include "shobjidl.h"
#include "MyFriendSerializer.h"
#include "RowHeaderColumn.h"
#include "FileNameColumn.h"
#include "FileIconColumn.h"
#include "FileExtColumn.h"
#include "FileSizeColumn.h"
#include "FileLastWriteColumn.h"
#include "MyXMLSerializer.h"

#define WM_UAHMEASUREMENUITEM 0x0094

class CCellEventArgs;
class CShellFile;
class CShellFolder;
class CFileDragger;
class CShowHideColumnMenuItem;

class CFilerGridViewBase :public CGridView
{
protected:
	//Columns
	std::shared_ptr<CFileNameColumn> m_pNameColumn;

	//HeaderMenuItems
	std::vector<std::shared_ptr<CShowHideColumnMenuItem>> m_headerMenuItems;
public:
	//Constructor
	CFilerGridViewBase(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp);
	virtual ~CFilerGridViewBase();
	//getter
	std::shared_ptr<FilerGridViewProperty> GetFilerGridViewPropPtr() { return std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp); }

	virtual bool HasSheetCell()override { return false; }
	virtual bool IsVirtualPage()override { return true; }

	/**********/
	/* Signal */
	/**********/
	std::function<void(CMenu&)> AddCustomContextMenu;
	std::function<bool(int, CComPtr<IShellFolder>, std::vector<PITEMID_CHILD>)> ExecCustomContextMenu;

	virtual void OnCellLButtonDblClk(CellEventArgs& e);
	virtual void Open(std::shared_ptr<CShellFile>& spFile);
	virtual void OpenFile(std::shared_ptr<CShellFile>& spFile);
	virtual void OpenFolder(std::shared_ptr<CShellFolder>& spFile) = 0;

	index_vector<std::shared_ptr<CRow>>::const_iterator FindIfRowIterByFileNameExt(const std::wstring& fileNameExt);

	virtual bool CutToClipboard();
	virtual bool CopyToClipboard();
	virtual bool PasteFromClipboard();
	virtual bool DeleteSelectedFiles(){ return false; }
	/******************/
	/* Window Message */
	/******************/
	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnCommandCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	virtual LRESULT OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
	virtual LRESULT OnCommandPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
	virtual LRESULT OnCommandDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;

	/*****************/
	/* State Message */
	/*****************/
	virtual void Normal_KeyDown(const KeyDownEvent& e) override;
	bool Edit_Guard_KeyDownWithNormal(const KeyDownEvent& e) override;
	bool Edit_Guard_KeyDownWithoutNormal(const KeyDownEvent& e) override;

};