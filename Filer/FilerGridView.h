#pragma once

#include "GridView.h"
#include "IDLPtr.h"
#include "Resource.h"

#include "GridViewProperty.h"

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

#define REGISTER_POLYMORPHIC_RELATION(Base, Derived)\
CSerializer::s_dynamicSerializeMap.insert(\
	std::make_pair(typeid(Derived).name(), [](CSerializer* se, MSXML2::IXMLDOMElementPtr pElem, void* ptr) {\
	se->SerializeValue(*(dynamic_cast<Derived*>(static_cast<Base*>(ptr))), pElem);\
}));\
CDeserializer::s_dynamicDeserializeMap.insert(\
	std::make_pair(typeid(Derived).name(), [](CDeserializer* se, MSXML2::IXMLDOMElementPtr pElem, void* ptr) {\
	se->DeserializeElement(*(dynamic_cast<Derived*>(static_cast<Base*>(ptr))), pElem);\
}));\
CDeserializer::s_dynamicMakeSharedMap.insert(\
	std::make_pair(typeid(Derived).name(), [this]()->std::shared_ptr<void> {\
	return std::make_shared<Derived>();\
}))

class CCellEventArgs;
class CShellFile;
class CShellFolder;
class CFileDragger;
class CIcon;
class CDirectoryWatcher;
class CShowHideColumnMenuItem;

namespace FilerColumn{
	enum Type{
		Icon,
		Path,
		Name,
		Ext,
		Type,
		Size,
		CreationTime,
		LastAccessTime,
		LastWriteTime,
	};
};

class CFilerGridView:public CGridView
{
private:	

	//std::vector<std::shared_ptr<CShellFile>> m_vpFile;
	std::shared_ptr<CDirectoryWatcher> m_spWatcher;

	CComPtr<IShellFolder> m_pDesktopShellFolder;
	std::shared_ptr<CShellFolder> m_spFolder;

	std::wstring m_initPath;

	//For Drag & Drop 
	CComPtr<IDropTarget> m_pDropTarget;
	CComPtr<IDropSource> m_pDropSource;
	CComPtr<IDragSourceHelper> m_pDragSourceHelper;

	//For Context Menu
	HMENU m_hNewMenu;

	CComPtr<IContextMenu2> m_pcmNew2;
	CComPtr<IContextMenu3> m_pcmNew3;

	CComPtr<IContextMenu2> m_pcm2;
	CComPtr<IContextMenu3> m_pcm3;

	//For DirectoryWatch
	std::wstring m_oldName;

	//For New
	bool m_bNewFile = false;
	//Columns
	std::shared_ptr<CFileNameColumn> m_pNameColumn;

	//HeaderMenuItems
	std::vector<std::shared_ptr<CShowHideColumnMenuItem>> m_headerMenuItems;

	//Remember past filter
	std::unordered_map<std::wstring, std::unordered_map<std::shared_ptr<CColumn>, std::wstring>> m_filterMap;

public:
	CFilerGridView(std::shared_ptr<CGridViewProperty> spGridViewProrperty);
	virtual ~CFilerGridView(){}
	//getter
	std::shared_ptr<CShellFolder>& GetFolder() { return m_spFolder; }

	//signal
	boost::signals2::signal<void(std::shared_ptr<CShellFolder>&)> FolderChanged;
	std::function<void(CMenu&)> AddCustomContextMenu;
	std::function<bool(int, CComPtr<IShellFolder>, std::vector<PITEMID_CHILD>)> ExecCustomContextMenu;

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDirectoryWatch(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	//virtual LRESULT OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnCommandCut(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandCopy(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled) override;
	virtual LRESULT OnCommandPaste(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled) override;
	virtual LRESULT OnCommandDelete(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled) override;

	virtual HRESULT OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual void OnKeyDown(const KeyDownEvent& e)override;
	virtual void OnBkGndLButtondDblClk(const LButtonDblClkEvent& e) override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;

	void OnCellLButtonDblClk(CellEventArgs& e);

	void Open(std::shared_ptr<CShellFile>& spFile);
	void OpenFile(std::shared_ptr<CShellFile>& spFile);
	void OpenFolder(std::shared_ptr<CShellFolder>& spFolder);


	string_type GetPath()const;
	void SetPath(const string_type& path);

	void Drag();
	BOOL SetDragImage(CIDLPtr pFirstIdl, CComPtr<IDragSourceHelper> pDragSourceHelper, IDataObject *pDataObject);

	RowDictionary::const_iterator FindIfRowIterByFileNameExt(const std::wstring& fileNameExt);

	bool NewFolder();
	bool CutToClipboard();
	bool CopyToClipboard();
	bool PasteFromClipboard();
	bool Delete();

	bool CopyTo(CComPtr<IShellItem2> pDestItem);
	bool MoveTo(CComPtr<IShellItem2> pDestItem);

	std::vector<LPITEMIDLIST> GetSelectedLastPIDLVector();
	std::vector<LPITEMIDLIST> GetSelectedAbsolutePIDLVector();

private:
	//Drag & Drop
	void Dropped(IDataObject *pDataObj, DWORD dwEffect);

	//DirectoryWatch action
	void Added(const std::wstring& fileName);
	void Modified(const std::wstring& fileName);
	void Removed(const std::wstring& fileName);
	void Renamed(const std::wstring& oldName, const std::wstring& newName);


	void InsertDefaultRowColumn();

	void ShowShellContextMenu(HWND hWnd, CPoint ptScreen, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl, bool hasNew = false);
	
	bool InvokeNewShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf);
	bool InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl);

public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(CColumn, CParentRowHeaderColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileIconColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileNameColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileExtColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn);

		std::vector<ColumnData> columns;
		for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
			columns.push_back(*iter);
		}
		ar("Columns", columns);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(CColumn, CParentRowHeaderColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileIconColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileNameColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileExtColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn);

		std::vector<ColumnData> columns;
		for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
			columns.push_back(*iter);
		}
		ar("Columns", columns);

		if (m_columnAllDictionary.empty()) {

			for (auto& col : columns) {
				if (auto p = std::dynamic_pointer_cast<CFileNameColumn>(col.DataPtr)) {
					m_pNameColumn = p;
				}
				col.DataPtr->SetSheetPtr(this);
				InsertColumnNotify(col.Index, col.DataPtr, false);
			}
		}
	}

};