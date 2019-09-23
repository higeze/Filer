#include "ThreadSafeKnownFolderManager.h"
#include "ShellFunction.h"

namespace shell
{
	CThreadSafeKnownFolderManager::CThreadSafeKnownFolderManager()
	{
		Update();
	}
	void CThreadSafeKnownFolderManager::Update()
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_knownFolderIdIdlMap.clear();

		CComPtr<IShellFolder> pDesktopFolder;
		::SHGetDesktopFolder(&pDesktopFolder);

		CComPtr<IKnownFolderManager> pMgr;
		if (FAILED(pMgr.CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER))) { return; }



		KNOWNFOLDERID* pknownid;
		UINT count(0);
		if (FAILED(pMgr->GetFolderIds(&pknownid, &count))) { return; }
		for (UINT i = 0; i < count; i++) {
			CComPtr<IKnownFolder> pFolder;
			if (SUCCEEDED(pMgr->GetFolder(pknownid[i], &pFolder))) {
				//IDL
				CIDL idl;
				pFolder->GetIDList(KF_FLAG_DEFAULT, idl.ptrptr());

				//Path Hint:pFolder->GetPath couldn't get GUID
				std::wstring path = shell::GetDisplayNameOf(pDesktopFolder, idl);

				if (pknownid[i] == FOLDERID_Desktop) {
					::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, idl.ptrptr());
					m_knownFolderIdIdlMap.insert_or_assign(path, std::make_tuple(pknownid[i], idl));
				} else {
					m_knownFolderIdIdlMap.emplace(path, std::make_tuple(pknownid[i], idl));
				}
			}
		}
	}
	bool CThreadSafeKnownFolderManager::IsExist(const std::wstring& path) const
	{
		return m_knownFolderIdIdlMap.find(path) != m_knownFolderIdIdlMap.end();
	}
}