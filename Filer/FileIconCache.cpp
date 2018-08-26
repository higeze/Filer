#include "FileIconCache.h"
#include "MyIcon.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "MyCom.h"
#include "KnownFolder.h"
#include <array>

CFileIconCache::CFileIconCache():m_ignoreSet({L".exe", L".ico", L".lnk"}){}

bool CFileIconCache::Exist(CShellFile* file)
{
	return
		(typeid(*file) == typeid(CShellFile) && m_iconMap.lock_find(file->GetExt()) != m_iconMap.end());
}


std::shared_ptr<CIcon> CFileIconCache::GetIcon(CShellFile* file)
{
	//Ext cached folder
	{
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
