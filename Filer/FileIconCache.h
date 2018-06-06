#pragma once
#include "IDLPtr.h"

class CIcon;
class CShellFile;
//class CIDLPtr;

class CFileIconCache
{
private:
	std::unordered_map<std::wstring, std::shared_ptr<CIcon>> m_iconMap;
	std::unordered_map<std::wstring, std::shared_ptr<CIcon>> m_knownIconMap;
	std::unordered_set<std::wstring> m_ignoreSet;
	std::shared_ptr<CIcon> m_defaultIcon;

public:
	CFileIconCache();

	bool Exist(CShellFile* file)const;
	std::shared_ptr<CIcon> GetIcon(CShellFile* file);
	std::shared_ptr<CIcon> GetDefaultIcon();

};
