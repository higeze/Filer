#pragma once
#include "IDL.h"
#include "KnownFolder.h"
#include <mutex>

class CIcon;
class CShellFile;
//class CIDLPtr;

template<typename Key, typename Value>
class lockable_unordered_map : public std::unordered_map<Key, Value>
{
private:
	std::mutex m_mtx;
public:

	auto lock_emplace(std::pair<Key, Value>& pair)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		return emplace(pair);
	}

	auto lock_insert(std::pair<Key, Value>& pair) 
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		return insert(pair);
	}

	auto lock_insert_or_assign(const Key& key, Value&& value)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		return insert_or_assign(key, value);
	}


	void lock_foreach(std::function<void(std::pair<Key, Value>&)> fun)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		for (auto& pair : *this)
		{
			fun(pair);
		}
	}

	auto lock_find(const Key& key)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		return find(key);
	}

};

class CFileIconCache
{
private:
	static CKnownFolderManager s_knownFolderManager;
	lockable_unordered_map<std::wstring, std::shared_ptr<CIcon>> m_iconMap;
	lockable_unordered_map<std::wstring, std::shared_ptr<CIcon>> m_knownIconMap;
	std::unordered_set<std::wstring> m_ignoreSet;
	std::shared_ptr<CIcon> m_defaultIcon;
	std::mutex m_mtx;

public:
	CFileIconCache();

	bool Exist(CShellFile* file);
	std::shared_ptr<CIcon> GetIcon(CShellFile* file);
	std::shared_ptr<CIcon> GetDefaultIcon();

};
