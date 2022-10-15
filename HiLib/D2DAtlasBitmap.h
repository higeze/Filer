#pragma once
#include "IDL.h"
#include "Direct2DWrite.h"
#include <future>
#include <mutex>
#include <unordered_set>
#include "getter_macro.h"

class CIcon;
class CShellFile;
//class CIDLPtr;

template<typename Key, typename Value>
class lockable_unordered_map : public std::unordered_map<Key, Value>
{
private:

	std::mutex m_mtx;
public:

	auto lock_emplace(const std::pair<Key, Value>& pair)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		return this->emplace(pair);
	}

	auto lock_insert(const std::pair<Key, Value>& pair) 
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		return this->insert(pair);
	}

	auto lock_insert_or_assign(const Key& key, const Value&& value)
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		return this->insert_or_assign(key, value);
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
		return this->find(key);
	}

};

class CD2DAtlasBitmapBase
{
protected: 
	CDirect2DWrite* m_pDirect;
	std::mutex m_mtx;
	CSizeU m_size;

	CComPtr<ID2D1Bitmap> m_pAtlasBitmap;
	virtual void LoadAtlasBitmapPtr();
	virtual void ClearAtlasBitmapPtr();
	virtual const CComPtr<ID2D1Bitmap>& GetAtlasBitmapPtr();
	virtual void DrawAtlasBitmap(const CRectF& dstRect, const CRectU& srcRect);
	virtual void DrawAtlasBitmap(const CRectF& dstRect, const CRectF& srcRect);
public:
	CD2DAtlasBitmapBase(CDirect2DWrite* pDirect, const CSizeU& size);

};

class CD2DAtlasFixedSizeBitmap: public CD2DAtlasBitmapBase
{
protected:
	CSizeU m_oneSize;
	std::unordered_map<std::wstring, CRectU> m_map;
	std::vector<std::future<void>> m_futures;
	int m_indexSize = 0;
	CRectU GetPlacement(const int& index);
public:
	CD2DAtlasFixedSizeBitmap(CDirect2DWrite* pDirect, const CSizeU& atlasBitmapSize, const CSizeU& oneBitmapSize );
	void Add(const std::wstring& key, const CComPtr<ID2D1Bitmap>& pBitmap);
	void AddAsync(const std::wstring& key, std::function<CComPtr<ID2D1Bitmap>()>&& fun, std::function<void()>&& funupd, const std::wstring& defKey);
	bool Exist(const std::wstring& key);
	void Clear();
	void DrawBitmapByKey(const std::wstring& key, const CRectF& dstRect);
};

