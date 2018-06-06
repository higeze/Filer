#include "FileIconCache.h"
#include "MyIcon.h"
#include "ShellFile.h"
#include "MyCom.h"

CFileIconCache::CFileIconCache():m_ignoreSet({L".exe", L".ico"})
{
	CCoInitializer coinit;
	CComPtr<IKnownFolderManager> pMgr;
	HRESULT hr = NULL;
	if (FAILED(pMgr.CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER))) { return; }
	KNOWNFOLDERID* pknownid;
	UINT count;
	if (FAILED(pMgr->GetFolderIds(&pknownid, &count))) { return; }
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);
	for (UINT i = 0; i < count; i++) {
		CComPtr<IKnownFolder> pFolder;
		if (SUCCEEDED(pMgr->GetFolder(pknownid[i], &pFolder))) {

			LPITEMIDLIST pidl;
			pFolder->GetIDList(NULL, &pidl);
			STRRET strret;
			pDesktop->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strret);
			std::wstring path = STRRET2WSTR(strret, pidl);
			m_knownIconMap.emplace(std::make_pair(path, std::shared_ptr<CIcon>(nullptr)));
			::ILFree(pidl);
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

bool CFileIconCache::Exist(CShellFile* file)const
{
	std::wstring ext(file->GetExt());
	if (ext.empty()) { ext = L"dir"; }
	return
		m_knownIconMap.find(file->GetPath()) != m_knownIconMap.end() ||
		m_iconMap.find(ext) != m_iconMap.end();
}


std::shared_ptr<CIcon> CFileIconCache::GetIcon(CShellFile* file)
{
	//Known folder
	{
		auto iter = m_knownIconMap.find(file->GetPath());
		if (iter != m_knownIconMap.end()) {
			if (!(iter->second)) {
				SHFILEINFO sfi = { 0 };
				::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)file->GetAbsolutePidl(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
				
				iter->second = std::make_shared<CIcon>(sfi.hIcon);
			}
			return iter->second;
		}
	}

	//Ext cached folder
	{
		std::wstring ext(file->GetExt());
		if (ext.empty()) { ext = L"dir"; }
		auto iter = m_iconMap.find(ext);
		if (iter != m_iconMap.end()) {
			return iter->second;
		} else {
			SHFILEINFO sfi = { 0 };
			::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)file->GetAbsolutePidl(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
			auto icon = std::make_shared<CIcon>(sfi.hIcon);

			if (m_ignoreSet.find(ext) == m_ignoreSet.end()) {
				m_iconMap.emplace(ext, icon);
			}
			return icon;
		}
	}



	//

	//std::wstring ext;
	//bool isNormalFolder(!(file->GetAttributes() & SFGAO_SYSTEM) && (file->GetAttributes() & SFGAO_FOLDER));
	//if (isNormalFolder) {
	//	ext = L"dir";
	//} else {
	//	ext = file->GetExt();
	//}
	//auto iter = m_iconMap.find(ext);
	//if (iter != m_iconMap.end()) {
	//	return iter->second;
	//} else {
	//	SHFILEINFO sfi = { 0 };
	//	::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)file->GetAbsolutePidl(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	//	auto icon = std::make_shared<CIcon>(sfi.hIcon);
	//	if (isNormalFolder ||
	//		//!file->GetExt().empty() &&
	//		m_ignoreSet.find(ext) == m_ignoreSet.end()) {
	//		m_iconMap.emplace(ext, icon);
	//	}
	//	return icon;
	//}
}

std::shared_ptr<CIcon> CFileIconCache::GetDefaultIcon()
{
	if (!m_defaultIcon) {
		HINSTANCE hDll = ::LoadLibrary(L"SHELL32.dll");
		m_defaultIcon = std::make_shared<CIcon>(::LoadIcon(hDll, MAKEINTRESOURCE(1)));
	}
	return m_defaultIcon;

}
