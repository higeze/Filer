#include "FileIconCache.h"
#include "MyIcon.h"
#include "ShellFile.h"

CFileIconCache::CFileIconCache():m_ignoreSet({L".exe", L".ico"}){}

bool CFileIconCache::Exist(CShellFile* file)const
{
	return 
		m_iconMap.find(file->GetExt()) != m_iconMap.end();//In icon cache
}

std::shared_ptr<CIcon> CFileIconCache::GetIcon(CShellFile* file)
{
	auto iter = m_iconMap.find(file->GetExt());
	if (iter != m_iconMap.end()) {
		return iter->second;
	} else {
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)file->GetAbsolutePidl(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
		auto icon = std::make_shared<CIcon>(sfi.hIcon);
		if (!file->IsShellFolder() &&
			!file->GetExt().empty() &&
			m_ignoreSet.find(file->GetExt()) == m_ignoreSet.end()) {
			m_iconMap.emplace(file->GetExt(), icon);
		}
		return icon;
	}
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
