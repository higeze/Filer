#pragma once

#include "GridView.h"
#include "IDLPtr.h"
#include "Resource.h"

#include "DirectoryWatcher.h"
#include "GridViewProperty.h"

#include "shobjidl.h"

class CCellEventArgs;
class CShellFile;
class CShellFolder;
class CFileDragger;
class CIcon;
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
	CDirectoryWatcher m_watcher;
	CComPtr<IContextMenu3> m_pcm3;

	CComPtr<IShellFolder> m_pDesktopShellFolder;
	std::shared_ptr<CShellFolder> m_spFolder;

	std::wstring m_initPath;

	CComPtr<IDropTarget> m_pDropTarget;
	CComPtr<IDropSource> m_pDropSource;
	CComPtr<IDragSourceHelper> m_pDragSourceHelper;

public:
	CFilerGridView(std::wstring initPath, std::shared_ptr<CGridViewProperty> spGridViewProrperty);
	virtual ~CFilerGridView(){}
	//signal
	boost::signals2::signal<void(std::shared_ptr<CShellFolder>&)> FolderChanged;
	static UINT WM_CHANGED;

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnFileChanged(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnCommandCut(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandCopy(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled) override;
	virtual LRESULT OnCommandPaste(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled) override;
	virtual LRESULT OnCommandDelete(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled) override;
	virtual LRESULT OnInitMenuPopUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDrawItem(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnMeasureItem(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	virtual void OnKeyDown(KeyEventArgs& e)override;
	void OnCellLButtonDblClk(CellEventArgs& e);
	virtual void OnContextMenu(ContextMenuEventArgs& e) override;

	void Open(std::shared_ptr<CShellFile>& spFile);
	void OpenFile(std::shared_ptr<CShellFile>& spFile);
	void OpenFolder(std::shared_ptr<CShellFolder>& spFolder);

	string_type GetPath()const;
	void SetPath(const string_type& path);

	void Drag();
	BOOL SetDragImage(CIDLPtr pFirstIdl, CComPtr<IDragSourceHelper> pDragSourceHelper, IDataObject *pDataObject);

	std::shared_ptr<CFileDragger> m_spFileDragger;

private:
	void InsertDefaultRowColumn();
	void OnShellCommand(LPCSTR lpVerb);
	void ShowShellContextMenu(HWND hWnd, CPoint ptScreen, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl);
	void InvokeShellCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl);
};