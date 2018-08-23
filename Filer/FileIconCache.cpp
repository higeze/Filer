#include "FileIconCache.h"
#include "MyIcon.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "MyCom.h"
#include "KnownFolder.h"
#include <array>

//CKnownFolderManager CFileIconCache::s_knownFolderManager;


CFileIconCache::CFileIconCache():m_ignoreSet({L".exe", L".ico", L".lnk"})
{
	//std::array<wchar_t, 64> logicalDrives;

	//if (::GetLogicalDriveStrings(64, logicalDrives.data())) {
	//	wchar_t* p;
	//	for (p = logicalDrives.data(); *p != L'\0';p+=lstrlen(p)+1) {
	//		m_driveIconMap.lock_emplace(std::make_pair(*p + std::wstring(L":\\"), std::shared_ptr<CIcon>(nullptr)));
	//	}
	//}

	//std::vector<CKnownFolder> knownFolders(s_knownFolderManager.GetKnownFolders());
	//for (auto kf : knownFolders) {
	//	m_knownIconMap.lock_emplace(std::make_pair(kf.GetPath(), std::shared_ptr<CIcon>(nullptr)));
	//}

	//CCoInitializer coinit;
	//CComPtr<IKnownFolderManager> pMgr;
	//HRESULT hr = NULL;
	//if (FAILED(pMgr.CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER))) { return; }
	//KNOWNFOLDERID* pknownid;
	//UINT count;
	//if (FAILED(pMgr->GetFolderIds(&pknownid, &count))) { return; }
	//CComPtr<IShellFolder> pDesktop;
	//::SHGetDesktopFolder(&pDesktop);
	//for (UINT i = 0; i < count; i++) {
	//	CComPtr<IKnownFolder> pFolder;
	//	if (SUCCEEDED(pMgr->GetFolder(pknownid[i], &pFolder))) {

	//		LPITEMIDLIST pidl;
	//		pFolder->GetIDList(NULL, &pidl);
	//		STRRET strret;
	//		pDesktop->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strret);
	//		std::wstring path = STRRET2WSTR(strret, pidl);
	//		m_knownIconMap.lock_emplace(std::make_pair(path, std::shared_ptr<CIcon>(nullptr)));
	//		::ILFree(pidl);
	//		//This way couldn't get desktop, computer, etc path as CLSID
	//		//LPWSTR pPath;
	//		//pFolder->GetPath(NULL, &pPath);
	//		//if (pPath) {
	//		//	m_knownIconMap.emplace(std::make_pair(std::wstring(pPath), std::shared_ptr<CIcon>(nullptr)));
	//		//} else {

	//		//}
	//		//::CoTaskMemFree(pPath);
	//	}
	//}
}

bool CFileIconCache::Exist(CShellFile* file)
{
	return
//		(m_driveIconMap.lock_find(file->GetPath()) != m_driveIconMap.end()) ||
//		(typeid(*file) == typeid(CShellFolder) && m_iconMap.lock_find(L"dir") != m_iconMap.end()) ||
		(typeid(*file) == typeid(CShellFile) && m_iconMap.lock_find(file->GetExt()) != m_iconMap.end());
}


std::shared_ptr<CIcon> CFileIconCache::GetIcon(CShellFile* file)
{
	//Drive folder
	//if (file->GetExt().empty()) {
	//	::DebugBreak();
	//}
	//{
	//	auto iter = m_driveIconMap.lock_find(file->GetPath());
	//	if (iter != m_driveIconMap.end()) {
	//		if (!(iter->second)) {
	//			SHFILEINFO sfi = { 0 };
	//			::SHGetFileInfo((LPCTSTR)file->GetAbsoluteIdl().ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);			
	//			m_driveIconMap.lock_insert_or_assign(iter->first, std::make_shared<CIcon>(sfi.hIcon));
	//		}
	//		return iter->second;
	//	}
	//}

	//Ext cached folder
	{
//		std::wstring ext((file->GetExt().empty() && typeid(*file)==typeid(CShellFolder)) ? L"dir" : file->GetExt());
		auto ext(file->GetExt());
		auto iter = m_iconMap.lock_find(ext);
		if (iter != m_iconMap.end()) {
			return iter->second;
		} else {
			SHFILEINFO sfi = { 0 };
			::SHGetFileInfo((LPCTSTR)file->GetAbsoluteIdl().ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
			auto icon = std::make_shared<CIcon>(sfi.hIcon);

			if (!ext.empty() && m_ignoreSet.find(ext) == m_ignoreSet.end()) {
				m_iconMap.lock_emplace(std::make_pair(ext, icon));
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

std::shared_ptr<CIcon> CFileIconCache::GetFolderIcon()
{
	if (!m_folderIcon) {
		auto folder = CKnownFolderManager::GetInstance()->GetKnownFolderById(FOLDERID_ProgramFiles);
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo((LPCTSTR)(folder->GetAbsoluteIdl().ptr()), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
		m_folderIcon = std::make_shared<CIcon>(sfi.hIcon);
	}
	return m_folderIcon;
}
