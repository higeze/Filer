#pragma once
#include "IDL.h"
#include "Direct2DWrite.h"
#include <mutex>
#include <unordered_set>

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

namespace d2dw
{

	class CFileIconCache
	{
	private:
		bool m_isWin10;
		CDirect2DWrite* m_pDirect;
		std::unordered_set<std::wstring> m_excludeExtSet;
		lockable_unordered_map<std::wstring, CComPtr<ID2D1Bitmap>> m_extMap;
		lockable_unordered_map <std::wstring, CComPtr<ID2D1Bitmap>> m_pathMap;
		CComPtr<ID2D1Bitmap> m_defaultIconBmp;
		CIcon GetIcon(const CIDL& absoluteIDL);
		CComPtr<ID2D1Bitmap> GetBitmapFromIcon(const CIcon& icon);

	public:
		CFileIconCache(CDirect2DWrite* pDirect);
		CFileIconCache() = default;

		CComPtr<ID2D1Bitmap> GetFileIconBitmap(const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, std::function<void()> updated = nullptr);
		CComPtr<ID2D1Bitmap> GetDefaultIconBitmap();
		void Clear();
	};

}
