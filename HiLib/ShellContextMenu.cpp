#include "ShellContextMenu.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "Debug.h"

/*********************/
/* CShellContextMenu */
/* Static Functions  */
/*********************/

CComPtr<IContextMenu3> CShellContextMenu::SetUpNewContextMenues(const CMenu& menu, const CIDL& idl)
{
	//New Context Menu {D969A300-E7FF-11d0-A93B-00A0C90F2719},
	while (true) {
		CComPtr<IUnknown> puk;
		CComPtr<IContextMenu3> pcmNew3;
		//CLSID clsid = { 0xd969a300, 0xe7ff, 0x11d0,{ 0xa9, 0x3b, 0x00, 0xa0, 0xc9, 0x0f, 0x27, 0x19 } };
		CLSID clsid;
		CLSIDFromString(CComBSTR("{D969A300-E7FF-11d0-A93B-00A0C90F2719}"), &clsid);
		FAILED_BREAK(puk.CoCreateInstance(clsid));
		CComPtr<IShellExtInit> psei;
		FAILED_BREAK(puk->QueryInterface(IID_PPV_ARGS(&psei)));
		FAILED_BREAK(psei->Initialize(idl.ptr(), NULL, NULL));
		FAILED_BREAK(psei->QueryInterface(IID_PPV_ARGS(&pcmNew3)));
		FAILED_BREAK(pcmNew3->QueryContextMenu(menu, 0, SCRATCH_QCM_NEW, SCRATCH_QCM_LAST, CMF_NORMAL));
		
		return pcmNew3;
	}
	return nullptr;
}

CComPtr<IContextMenu3> CShellContextMenu::SetUpNormalContextMenues(
	const CMenu& menu, const CComPtr<IShellFolder>& folder, HWND hwndOwner, UINT cidl, LPCITEMIDLIST *apidl)
{
	while (1) {
		CComPtr<IContextMenu> pcm;
		CComPtr<IContextMenu3> pcm3;
		FAILED_BREAK(folder->GetUIObjectOf(hwndOwner, cidl, apidl, IID_IContextMenu, nullptr, (LPVOID*)&pcm));
		FAILED_BREAK(pcm->QueryInterface(IID_PPV_ARGS(&pcm3)));
		FAILED_BREAK(pcm3->QueryContextMenu(menu, 0, SCRATCH_QCM_FIRST, SCRATCH_QCM_LAST, ::IsKeyDown(VK_SHIFT) ? CMF_NORMAL | CMF_EXTENDEDVERBS : CMF_NORMAL));
		return pcm3;
	}
	return nullptr;
}

bool CShellContextMenu::InvokeShellCommand(HWND hWnd, POINT pt, UINT id, const CComPtr<IContextMenu3>& pcm, const CComPtr<IContextMenu3>& pcmNew)
{
	CMINVOKECOMMANDINFOEX info = { 0 };
	info.cbSize = sizeof(info);
	info.fMask = /*CMIC_MASK_UNICODE |*/ CMIC_MASK_PTINVOKE;
	info.hwnd = hWnd;
	info.nShow = SW_SHOWNORMAL;
	info.ptInvoke = pt;

	if (pcmNew && id >= SCRATCH_QCM_NEW) {
		info.lpVerb = MAKEINTRESOURCEA(id - SCRATCH_QCM_NEW);
		info.lpVerbW = MAKEINTRESOURCEW(id - SCRATCH_QCM_NEW);
		return SUCCEEDED(pcmNew->InvokeCommand((LPCMINVOKECOMMANDINFO)&info));
	}else if(pcm) {
		info.lpVerb = MAKEINTRESOURCEA(id - SCRATCH_QCM_FIRST);
		info.lpVerbW = MAKEINTRESOURCEW(id - SCRATCH_QCM_FIRST);
		return SUCCEEDED(pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&info));
	}
	return false;
}

bool CShellContextMenu::InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, const std::shared_ptr<CShellFolder>& folder)
{
	std::vector<PITEMID_CHILD> vpIdl = {folder->GetAbsoluteIdl().FindLastID()};

	do {
		CMenu menu(::CreatePopupMenu());
		FALSE_BREAK(menu);
		CComPtr<IContextMenu3> pcm = SetUpNormalContextMenues(menu, folder->GetParentShellFolderPtr(), hWnd, vpIdl.size(), (LPCITEMIDLIST*)(vpIdl.data()));

		CMINVOKECOMMANDINFO cmi = {0};
		cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
		cmi.fMask = CMIC_MASK_NOASYNC;
		cmi.hwnd = hWnd;
		cmi.lpVerb = lpVerb;
		cmi.nShow = SW_SHOWNORMAL;

		return SUCCEEDED(pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi));

	} while (false);
	return false;
}

bool CShellContextMenu::InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, const std::vector<std::shared_ptr<CShellFile>>& files)
{
	std::vector<PITEMID_CHILD> vpIdl;
	for (auto file : files) {
		vpIdl.push_back(file->GetAbsoluteIdl().FindLastID());
	}

	do {
		CComPtr<IContextMenu> pcm;
		FAILED_BREAK(files[0]->GetParentShellFolderPtr()->GetUIObjectOf(hWnd, vpIdl.size(),(LPCITEMIDLIST*)(vpIdl.data()),IID_IContextMenu,nullptr,(LPVOID *)&pcm));
		CMenu menu(::CreatePopupMenu());
		FALSE_BREAK(menu);
		FAILED_BREAK(pcm->QueryContextMenu(menu, 0, 1, 0x7FFF, CMF_NORMAL));

		CMINVOKECOMMANDINFO cmi = {0};
		cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
		cmi.fMask = CMIC_MASK_NOASYNC;
		cmi.hwnd = hWnd;
		cmi.lpVerb = lpVerb;
		cmi.nShow = SW_SHOWNORMAL;

		return SUCCEEDED(pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi));

	} while (false);
	return false;
}

bool CShellContextMenu::InvokeNewShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb,  const std::shared_ptr<CShellFolder>& folder)
{
	CMenu menu(::CreatePopupMenu());
	CComPtr<IContextMenu3> pcmNew3 = SetUpNewContextMenues(menu, folder->GetAbsoluteIdl());

	CMINVOKECOMMANDINFO cmi = { 0 };
	cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
	cmi.hwnd = hWnd;
	cmi.lpVerb = lpVerb;
	cmi.nShow = SW_SHOWNORMAL;
	return SUCCEEDED(pcmNew3->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi));
}

/*********************/
/* CShellContextMenu */
/*********************/

int CShellContextMenu::PopupFolder(CWnd* pWnd, const CPoint& pt, const std::shared_ptr<CShellFolder>& folder)
{
	int id = 0;
	do {
		std::vector<PITEMID_CHILD> vPidl = {folder->GetAbsoluteIdl().FindLastID()};

		//OnHandleMenuMsg
		pWnd->AddMsgHandler(WM_INITMENUPOPUP, &CShellContextMenu::OnHandleMenuMsg, this);

		//Create Menu
		CMenu menu(::CreatePopupMenu());
		FALSE_BREAK(menu);

		//New Context Menu {D969A300-E7FF-11d0-A93B-00A0C90F2719},
		FALSE_BREAK((m_pcmNew3 = SetUpNewContextMenues(menu, folder->GetAbsoluteIdl())));

		//Normal Context Menu
		FALSE_BREAK((m_pcm3 = SetUpNormalContextMenues(menu, folder->GetParentFolderPtr()->GetShellFolderPtr(), pWnd->m_hWnd, vPidl.size(), (LPCITEMIDLIST*)vPidl.data())));

		//Investigate New menu
		int itemCount = menu.GetMenuItemCount();
		for (auto i = itemCount - 1; i != 0; i--) {
			std::wstring verb;
			MENUITEMINFO mii = {0};
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_SUBMENU | MIIM_TYPE;
			mii.dwTypeData = ::GetBuffer(verb, 256);
			mii.cch = 256;
			menu.GetMenuItemInfo(i, TRUE, &mii);

			//Get Submenu of New
			if (verb.find(L"V‹Kì¬") != std::wstring::npos) {
				m_hNewMenu = mii.hSubMenu;
			}
		}

		//Add Menu
		for (const auto& item : m_items) {
			menu.InsertMenuItemW(menu.GetMenuItemCount(), TRUE, &item->GetMII());
		}

		//Popup
		pWnd->SetForegroundWindow();
		id = menu.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pt.x, pt.y, pWnd->m_hWnd);

		//Invoke
		if (id) {
			//Execute Shell Command
			if (InvokeShellCommand(pWnd->m_hWnd, static_cast<tagPOINT>(pt), id, m_pcm3, m_pcmNew3)) {
				//Execute additional menu
			} else {
				auto iter = std::find_if(m_items.cbegin(), m_items.cend(),
					[id](const std::unique_ptr<CMenuItem2>& item) {
					return CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, item->Header->c_str()) == id;
				});

				if (iter != m_items.cend()) {
					(*iter)->Command.execute();
				}
			}
		}
		//OnHandleMenuMsg
		pWnd->RemoveMsgHandler(WM_INITMENUPOPUP);
	
	} while (false);

	//Release
	m_hNewMenu = NULL;
	if (m_pcmNew3) { m_pcmNew3.Release(); }
	if (m_pcm3) { m_pcm3.Release(); }
	return id;
}

int CShellContextMenu::PopupFiles(CWnd* pWnd, const CPoint& pt, const std::vector<std::shared_ptr<CShellFile>>& files)
{
	int id = 0;
	do {
		std::vector<PITEMID_CHILD> vPidl;
		for (const auto& file : files) {
			vPidl.push_back(file->GetAbsoluteIdl().FindLastID());
		}

		//Create Menu
		CMenu menu(::CreatePopupMenu());
		FALSE_BREAK(menu);

		//Normal Context Menu
		FALSE_BREAK((m_pcm3 = SetUpNormalContextMenues(menu, files[0]->GetParentShellFolderPtr(), pWnd->m_hWnd, vPidl.size(), (LPCITEMIDLIST*)vPidl.data())));

		//Add Menu
		for (const auto& item : m_items) {
			menu.InsertMenuItemW(menu.GetMenuItemCount(), TRUE, &item->GetMII());
		}

		//Popup
		pWnd->SetForegroundWindow();
		id = menu.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pt.x, pt.y, pWnd->m_hWnd);

		//Invoke
		if (id) {
			//Execute Shell Command
			if (InvokeShellCommand(pWnd->m_hWnd, static_cast<tagPOINT>(pt), id, m_pcm3, m_pcmNew3)) {
				//Execute additional menu
			} else {
				auto iter = std::find_if(m_items.cbegin(), m_items.cend(),
					[id](const std::unique_ptr<CMenuItem2>& item) {
					return CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, item->Header->c_str()) == id;
				});

				if (iter != m_items.cend()) {
					(*iter)->Command.execute();
				}
			}
		}
	} while (false);

	//Release
	if (m_pcm3) { m_pcm3.Release(); }
	return id;
}

HRESULT CShellContextMenu::OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ((m_pcmNew3) && wParam == (WPARAM)m_hNewMenu) {
		if (m_pcmNew3) {
			if (LRESULT lres; SUCCEEDED(m_pcmNew3->HandleMenuMsg2(uMsg, wParam, lParam, &lres))) {
				bHandled = TRUE;
					return lres;
			}
		} else {
			bHandled = FALSE;
			return 0;
		}
	}
	bHandled = FALSE;
	return 0;
}







