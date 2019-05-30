#pragma once

#include "GridView.h"
#include "IDL.h"
#include "Resource.h"

#include "FilerGridViewProperty.h"

#include "shobjidl.h"
#include "MyFriendSerializer.h"
#include "ParentRowHeaderColumn.h"
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
	virtual ~CFilerGridViewBase() {}
	//getter
	std::shared_ptr<FilerGridViewProperty> GetFilerGridViewPropPtr() { return std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp); }

	virtual bool HasSheetCell()override { return false; }
	virtual bool IsVirtualPage()override { return true; }

	//signal
	std::function<void(CMenu&)> AddCustomContextMenu;
	std::function<bool(int, CComPtr<IShellFolder>, std::vector<PITEMID_CHILD>)> ExecCustomContextMenu;

	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT OnCommandCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	virtual LRESULT OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
	virtual LRESULT OnCommandPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
	virtual LRESULT OnCommandDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;

	virtual void OnKeyDown(const KeyDownEvent& e)override;

	virtual void OnCellLButtonDblClk(CellEventArgs& e);
	void OpenFile(std::shared_ptr<CShellFile>& spFile);

	RowDictionary::const_iterator FindIfRowIterByFileNameExt(const std::wstring& fileNameExt);

	virtual bool CutToClipboard();
	virtual bool CopyToClipboard();
	virtual bool PasteFromClipboard();
	virtual bool Delete();
protected:
	virtual void InsertDefaultRowColumn();


public:
	//FRIEND_SERIALIZER
	//template <class Archive>
	//void save(Archive& ar)
	//{
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CParentRowHeaderColumn);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileNameColumn);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileExtColumn);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn);

	//	std::vector<ColumnData> columns;
	//	for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
	//		columns.push_back(*iter);
	//	}
	//	ar("Columns", columns);
	//}

	//template <class Archive>
	//void load(Archive& ar)
	//{
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CParentRowHeaderColumn, this);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileNameColumn, this);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileExtColumn, this);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn, this, GetFilerGridViewPropPtr()->FileSizeArgsPtr);
	//	REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn, this, GetFilerGridViewPropPtr()->FileTimeArgsPtr);

	//	std::vector<ColumnData> columns;
	//	for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
	//		columns.push_back(*iter);
	//	}
	//	ar("Columns", columns);

	//	if (m_columnAllDictionary.empty()) {

	//		for (auto& col : columns) {
	//			if (auto p = std::dynamic_pointer_cast<CFileNameColumn>(col.DataPtr)) {
	//				m_pNameColumn = p;
	//			} else if (auto p = std::dynamic_pointer_cast<CParentRowHeaderColumn>(col.DataPtr)) {
	//				m_pHeaderColumn = p;
	//			}
	//			InsertColumnNotify(col.Index, col.DataPtr, false);
	//		}
	//	}
	//}
};