#include "KnownFolder.h"
#include "MyCom.h"

CKnownFolder::CKnownFolder(CIDL absoluteIdl, std::wstring path)
	:m_idl(absoluteIdl), m_path(path) {}

CKnownFolderManager::CKnownFolderManager()
{
	CCoInitializer coinit;
	CComPtr<IKnownFolderManager> pMgr;
	if (FAILED(pMgr.CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER))) { return; }
	KNOWNFOLDERID* pknownid;
	UINT count(0);
	if (FAILED(pMgr->GetFolderIds(&pknownid, &count))) { return; }
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);
	for (UINT i = 0; i < count; i++) {
		CComPtr<IKnownFolder> pFolder;
		if (SUCCEEDED(pMgr->GetFolder(pknownid[i], &pFolder))) {

			CIDL idl;
			pFolder->GetIDList(NULL, idl.ptrptr());
			STRRET strret;
			pDesktop->GetDisplayNameOf(idl.ptr(), SHGDN_FORPARSING, &strret);
			std::wstring path = STRRET2WSTR(strret, idl.ptr());
			m_knownFolders.emplace_back(idl, path);
			//::ILFree(pidl);
			//This way couldn't get desktop, computer, etc path as CLSID
			//LPWSTR pPath;
			//pFolder->GetPath(NULL, &pPath);
			//if (pPath) {
			//	m_knownIconMap.emplace(std::make_pair(std::wstring(pPath), std::shared_ptr<CIcon>(nullptr)));
			//} else {

			//}
			//::CoTaskMemFree(pPath);
		}
	}

}

bool CKnownFolderManager::IsKnownFolder(CIDL& idl)
{
	return std::find_if(m_knownFolders.begin(), m_knownFolders.end(), [idl](const auto& folder)->bool {return folder.GetIDL() == idl; }) != m_knownFolders.end();
}
