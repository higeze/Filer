#pragma once

#include "GridView.h"
#include "IDLPtr.h"
#include "Resource.h"

#include "GridViewProperty.h"

#include "shobjidl.h"

#define WM_UAHMEASUREMENUITEM 0x0094

class CCellEventArgs;
class CShellFile;
class CShellFolder;
class CFileDragger;
class CIcon;
class CDirectoryWatcher;
class CFileNameColumn;
//class CDropTarget;
//class CDropSource;

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

	std::vector<std::shared_ptr<CShellFile>> m_vpFile;
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
	std::shared_ptr<CFileNameColumn> m_pNameColumn;

public:
	CFilerGridView(std::shared_ptr<CGridViewProperty> spGridViewProrperty);
	virtual ~CFilerGridView(){}
	//signal
	boost::signals2::signal<void(std::shared_ptr<CShellFolder>&)> FolderChanged;
	static UINT WM_CHANGED;

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDirectoryWatch(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
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
	//auto iter = std::find_if(m_rowAllDictionary.begin(), m_rowAllDictionary.end(),
	//	[&](const RowData& data)->bool {
	//	if (auto p = std::dynamic_pointer_cast<CFileRow>(data.DataPtr)) {
	//		return p->GetFilePointer()->GetName() == fileName;
	//	}
	//	return false;
	//});

private:

	void Added(const std::wstring& fileName);
	void Modified(const std::wstring& fileName);
	void Removed(const std::wstring& fileName);
	void Renamed(const std::wstring& oldName, const std::wstring& newName);


	void InsertDefaultRowColumn();
	void OnShellCommand(LPCSTR lpVerb);
	void ShowShellContextMenu(HWND hWnd, CPoint ptScreen, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl, bool hasNew = false);
	void InvokeShellCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl);
};