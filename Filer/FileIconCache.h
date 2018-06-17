#pragma once
#include "IDLPtr.h"
#include <mutex>

class CIcon;
class CShellFile;
//class CIDLPtr;

//template<typename Key, typename Value>
//class lockable_unordered_map : public std::unordered_map<Key, Value>
//{
//};

class CFileIconCache
{
private:
	std::unordered_map<std::wstring, std::shared_ptr<CIcon>> m_iconMap;
	std::unordered_map<std::wstring, std::shared_ptr<CIcon>> m_knownIconMap;
	std::unordered_set<std::wstring> m_ignoreSet;
	std::shared_ptr<CIcon> m_defaultIcon;
	std::mutex m_mtx;

public:
	CFileIconCache();

	bool Exist(CShellFile* file);
	std::shared_ptr<CIcon> GetIcon(CShellFile* file);
	std::shared_ptr<CIcon> GetDefaultIcon();

};
