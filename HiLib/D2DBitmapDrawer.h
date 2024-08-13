#pragma once
#include "D2DAtlasBitmap.h"
#include "D2DWTypes.h"
#include "shared_lock_property.h"
#include "getter_macro.h"

class CThreadPool;

template<class _Key>
class CD2DBitmapDrawer
{
protected:
	std::unique_ptr<CD2DAtlasBitmap<_Key>> m_pAtlasPrimaryBitmap;
	std::unique_ptr<CD2DAtlasBitmap<_Key>> m_pAtlasSecondaryBitmap;
	std::unique_ptr<CThreadPool> m_pThreadPool;


	shared_lock_property<_Key> m_curKey;
	shared_lock_property<_Key> m_curClipKey;
	shared_lock_property<_Key> m_curBlurKey;
	future_group<void> m_futureGroup;

public:

	const std::unique_ptr<CThreadPool>& GetThreadPoolPtr() const
	{
		return m_pThreadPool;
	}

	const std::unique_ptr<CD2DAtlasBitmap<_Key>>& GetAtlasPrimaryBitmap() const
	{ 
		return m_pAtlasPrimaryBitmap;
	}
	const std::unique_ptr<CD2DAtlasBitmap<_Key>>& GetAtlasSecondaryBitmap() const
	{ 
		return m_pAtlasSecondaryBitmap;
	}

public:
	CD2DBitmapDrawer()
		:m_pAtlasPrimaryBitmap(std::make_unique<CD2DAtlasBitmap<_Key>>(
			CSizeU(4096, 8196),
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_NONE,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))),
		m_pAtlasSecondaryBitmap(std::make_unique<CD2DAtlasBitmap<_Key>>(
			CSizeU(2048, 4096),
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_NONE,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))),
		m_pThreadPool(std::make_unique<CThreadPool>()){}

	virtual ~CD2DBitmapDrawer() = default;

	virtual void AddClipBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		std::function<void()> callback)
	{
		auto cancel = [this, key]()->bool {return key != m_curClipKey.get(); };
		do {
			TRUE_BREAK(cancel());

			auto bmp = key.GetClipBitmap(cancel);
			FALSE_BREAK(bmp);
			m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, bmp);
			callback();
			return;

		} while (1);

		m_pAtlasPrimaryBitmap->Erase(key);
	}

	bool DrawClipBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		const CPointF& dstPoint,
		std::function<void()> callback)
	{	
		if (!m_pAtlasPrimaryBitmap->Exist(key)) {
			m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, key.GetDefaultBitmap());
			m_curClipKey.set(key);
			m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0, 
				[this, pDirect, key, callback]()->void {AddClipBitmap(pDirect, key, callback); }));
			return false;
		} else {
			return m_pAtlasPrimaryBitmap->DrawBitmap(pDirect, key, dstPoint);
		}
	}

	bool DrawClipBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		const CRectF& dstRect,
		std::function<void()> callback)
	{	
		if (!m_pAtlasPrimaryBitmap->Exist(key)) {
			m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, key.GetDefaultBitmap());
			m_curClipKey.set(key);
			m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0, 
				[this, pDirect, key, callback]()->void {AddClipBitmap(pDirect, key, callback); }));
			return false;
		} else {
			return m_pAtlasPrimaryBitmap->DrawBitmap(pDirect, key, dstRect);
		}
	}

	virtual void AddBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		std::function<void()> callback)
	{
		auto cancel = [this, key]()->bool {return key != m_curKey.get(); };
		do {
			TRUE_BREAK(cancel());

			auto bmp = key.GetBitmap(cancel);
			FALSE_BREAK(bmp);
			m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, bmp);

			callback();
			return;
		} while (1);

		m_pAtlasPrimaryBitmap->Erase(key);
	}


	bool DrawBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		const CPointF& dstPoint,
		std::function<void()> callback)
	{
		if (!m_pAtlasPrimaryBitmap->Exist(key)) {
			m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, key.GetDefaultBitmap());
			m_curKey.set(key);
			m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0,
				[this, pDirect, key, callback]()->void {AddBitmap(pDirect, key, callback); }));
			return false;
		} else {
			return m_pAtlasPrimaryBitmap->DrawBitmap(pDirect, key, dstPoint);
		}
	}

	bool DrawBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		const CRectF& dstRect,
		std::function<void()> callback)
	{
		if (!m_pAtlasPrimaryBitmap->Exist(key)) {
			m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, key.GetDefaultBitmap());
			m_curKey.set(key);
			m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0,
				[this, pDirect, key, callback]()->void {AddBitmap(pDirect, key, callback); }));
			return false;
		} else {
			return m_pAtlasPrimaryBitmap->DrawBitmap(pDirect, key, dstRect);
		}
	}

	virtual void AddBlurBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		std::function<void()> callback)
	{
		auto cancel = [this, key]()->bool {return key != m_curBlurKey.get(); };
		do {
			TRUE_BREAK(cancel());

			auto bmp = key.GetBitmap(cancel);
			FALSE_BREAK(bmp);
			m_pAtlasSecondaryBitmap->AddOrAssign(pDirect, key, bmp);

			callback();
			return;
		} while (1);

		m_pAtlasSecondaryBitmap->Erase(key);
	}

	bool DrawBlurBitmap(
		const CDirect2DWrite* pDirect,
		const _Key& key,
		const CRectF& dstRect,
		std::function<void()> callback)
	{
		if (!m_pAtlasSecondaryBitmap->Exist(key)) {
			m_pAtlasSecondaryBitmap->AddOrAssign(pDirect, key, key.GetDefaultBitmap());
			m_curBlurKey.set(key);
			m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 10,
				[this, pDirect, key, callback]()->void {AddBlurBitmap(pDirect, key, callback); }));
			return false;
		} else {
			return m_pAtlasSecondaryBitmap->DrawBitmap(pDirect, key, dstRect);
		}

	}

	std::vector<_Key> FindPrimaryKeys(std::function<bool(const _Key&)>&& pred)
	{
		std::vector<_Key> keys(m_pAtlasPrimaryBitmap->Keys());
		std::vector<_Key> findKeys;
		std::copy_if(keys.cbegin(), keys.cend(), std::back_inserter(findKeys), pred);
		return findKeys;
	}

	void Clear()
	{
		m_pAtlasPrimaryBitmap->Clear();
		m_pAtlasSecondaryBitmap->Clear();
		m_futureGroup.clean_up();
	}
	void WaitAll()
	{
		m_futureGroup.wait_all();
	}
};