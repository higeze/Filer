#pragma once
#include <atlcom.h>
#include <ShlObj.h>
#include "MyMenu.h"
#include "IDL.h"

class CShellFile;
class CShellFolder;

//class CShellContextMenu
//{
//protected:
//	std::shared_ptr<IContextMenu> m_spContextMenu;
//public:
//	CShellContextMenu(LPCONTEXTMENU lpcm = nullptr);
//	void Attach(LPCONTEXTMENU lpcm);
//	~CShellContextMenu(void);
//
//	CMenu QueryContextMenu(UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
//};

class CShellContextMenu : public CContextMenu2
{
public:
	static const UINT SCRATCH_QCM_FIRST = 1;
	static const UINT SCRATCH_QCM_NEW = 600;//200,500 are used by system
	static const UINT SCRATCH_QCM_LAST = 0x7FFF;

	HMENU m_hNewMenu;
	CComPtr<IContextMenu3> m_pcmNew3;
	CComPtr<IContextMenu3> m_pcm3;
public:
	using CContextMenu2::CContextMenu2;
	virtual ~CShellContextMenu() {}
	//CShellContextMenu(const CShellContextMenu&) = default;
	CShellContextMenu(CShellContextMenu&&) = default;
	//CShellContextMenu& operator=(const CShellContextMenu&) = default;
	CShellContextMenu& operator=(CShellContextMenu&&) = default;

private:
	static CComPtr<IContextMenu3> SetUpNewContextMenues(const CMenu& menu, const CIDL& idl);
	static CComPtr<IContextMenu3> SetUpNormalContextMenues(
		const CMenu& menu, const CComPtr<IShellFolder>& folder, HWND hwndOwner, UINT cidl, LPCITEMIDLIST* apidl);
	static bool InvokeShellCommand(HWND hWnd, POINT pt, UINT id, const CComPtr<IContextMenu3>& pcm, const CComPtr<IContextMenu3>& pcmNew);
public:
	static bool InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, const std::shared_ptr<CShellFolder>& folder);
	static bool InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb,  const std::vector<std::shared_ptr<CShellFile>>& files);
	static bool InvokeNewShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, const std::shared_ptr<CShellFolder>& folder);

public:

	int PopupFolder(CWnd* pWnd, const CPoint& pt, const std::shared_ptr<CShellFolder>& folder);
	int PopupFiles(CWnd* pWnd, const CPoint& pt, const std::vector<std::shared_ptr<CShellFile>>& files);
	HRESULT OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};


