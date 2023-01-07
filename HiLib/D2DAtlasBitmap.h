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

#include <shared_mutex>


template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = std::allocator<std::pair<const _Kty, CRectU>>>
class CD2DAtlasBitmap
{
protected:
	using map_type = std::unordered_map<_Kty, CRectU, _Hasher, _Keyeq, _Alloc>;
	static const UINT s_size = 512;
	map_type m_map;
	CComPtr<ID2D1Bitmap1> m_pAtlasBitmap;
	std::vector<CRectU> m_rooms;
	std::shared_mutex m_mtx;

public:
	CD2DAtlasBitmap() { m_rooms.emplace_back(0, 0, s_size - 1, s_size - 1); }

	void DrawBitmap(const CDirect2DWrite* pDirect, const _Kty& key, const CRectF& dstRect)
	{
		std::shared_lock<std::shared_mutex> lock(m_mtx);
		
		typename map_type::const_iterator iter = m_map.find(key);
		if (iter != m_map.cend() && !(iter->second.IsRectNull())) {
			CRectF dstRc(
				std::round(dstRect.left),
				std::round(dstRect.top),
				std::round(dstRect.left + iter->second.Width() + 1),
				std::round(dstRect.top + iter->second.Height() + 1));
			CRectF srcRc(
				static_cast<FLOAT>(iter->second.left),
				static_cast<FLOAT>(iter->second.top),
				static_cast<FLOAT>(iter->second.right + 1),
				static_cast<FLOAT>(iter->second.bottom + 1));
			pDirect->GetD2DDeviceContext()->DrawBitmap(
				GetAtlasBitmapPtr(pDirect),
				dstRc,
				1.f,
				D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				srcRc);
		}
	}
	void AddOrAssign(const CDirect2DWrite* pDirect, const _Kty& key, const CComPtr<ID2D1Bitmap1>& pBitmap)
	{
		std::lock_guard<std::shared_mutex> lock(m_mtx);

		auto iter = m_map.find(key);
		if(pBitmap){
			CRectU newRect;
			CSizeU size(pBitmap->GetPixelSize());
			if (iter == m_map.cend() || iter->second.IsRectNull()) {
				newRect = FindRect(size);
				if (newRect.IsRectNull()) {
					pDirect->SaveBitmap(L"Test.png", GetAtlasBitmapPtr(pDirect));
					m_pAtlasBitmap.Release();
					m_map.clear();
					m_rooms.clear();
					m_rooms.emplace_back(0, 0, s_size - 1, s_size - 1);
					newRect = FindRect(size);
				}
			} else {
				newRect = iter->second;
			}
			D2D1_POINT_2U dstLeftTop{ newRect.left, newRect.top };
			D2D1_RECT_U srcRect{ 0, 0, size.width, size.height };

			FAILED_THROW(GetAtlasBitmapPtr(pDirect)->CopyFromBitmap(&dstLeftTop, pBitmap, &srcRect));
			m_map.insert_or_assign(key, newRect);
		} else {
			m_map.insert_or_assign(key, CRectU());
		}

		//if (pBitmap) {
		//	auto iter = m_map.find(key);
		//	if (iter == m_map.cend()) {
		//		UINT32 padding = 10;
		//		CSizeU size(pBitmap->GetPixelSize());
		//		CSizeU sizeWithPadding(size.width + padding * 2, size.height + padding * 2);
		//		CRectU newRectWithPadding = FindRect(sizeWithPadding);
		//		if (newRectWithPadding.IsRectNull()) {
		//			Save(pDirect, L"Test.png", GetAtlasBitmapPtr(pDirect));
		//			if (HRESULT hr = pDirect->GetD2DDeviceContext()->Flush(); FAILED(hr)){
		//				//::DebugBreak();
		//			}
		//			Clear();
		//			newRectWithPadding = FindRect(sizeWithPadding);
		//		}
		//		CRectU newRect(newRectWithPadding); newRect.DeflateRect(10);
		//		
		//		D2D1_POINT_2U dstLeftTop{ newRect.left, newRect.top };
		//		D2D1_RECT_U srcRect{ 0, 0, size.width, size.height };

		//		FAILED_THROW(GetAtlasBitmapPtr(pDirect)->CopyFromBitmap(&dstLeftTop, pBitmap, &srcRect));
		//		if (HRESULT hr = pDirect->GetD2DDeviceContext()->Flush(); FAILED(hr)){
		//			//::DebugBreak();
		//		}
		//		m_map.insert_or_assign(key, newRect);
		//	} else {
		//		
		//	}			
		//} else {
		//	m_map.insert_or_assign(key, CRectU());
		//}
	}
	bool Exist(const _Kty& key)
	{
		std::shared_lock<std::shared_mutex> lock(m_mtx);
		return m_map.find(key) != m_map.end();
	}

	//void Save(const CDirect2DWrite* pDirect, const std::wstring& path, const CComPtr<ID2D1Bitmap>& pBitmap)
	//{
	//	std::shared_lock<std::shared_mutex> lock(m_mtx);
	//	CComPtr<IWICStream> pStream;
	//	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateStream(&pStream));
	//	FAILED_THROW(pStream->InitializeFromFilename(path.c_str(), GENERIC_WRITE));

	//	CComPtr<IWICBitmapEncoder> pBmpEncoder;
	//	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateEncoder(GUID_ContainerFormatPng, NULL, &pBmpEncoder));
	//	FAILED_THROW(pBmpEncoder->Initialize(pStream, WICBitmapEncoderNoCache));

	//	CComPtr<IWICBitmapFrameEncode> pBmpFrameEncode;
	//	FAILED_THROW(pBmpEncoder->CreateNewFrame(&pBmpFrameEncode, NULL));
	//	FAILED_THROW(pBmpFrameEncode->Initialize(NULL));

	//	CComPtr<IWICImageEncoder> pImageEncoder;
	//	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateImageEncoder(pDirect->GetD2DDevice(), &pImageEncoder));
	//	FAILED_THROW(pImageEncoder->WriteFrame(pBitmap, pBmpFrameEncode, nullptr));

	//	FAILED_THROW(pBmpFrameEncode->Commit());
	//	FAILED_THROW(pBmpEncoder->Commit());
	//	FAILED_THROW(pStream->Commit(STGC_DEFAULT));
	//}

	void Clear()
	{
		std::lock_guard<std::shared_mutex> lock(m_mtx);
		m_pAtlasBitmap.Release();
		m_map.clear();
		m_rooms.clear();
		m_rooms.emplace_back(0, 0, s_size - 1, s_size - 1);
	}

private:	 
	const CComPtr<ID2D1Bitmap1>& GetAtlasBitmapPtr(const CDirect2DWrite* pDirect)
	{
		if (!m_pAtlasBitmap) {
			FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmap(
				CSizeU(s_size, s_size),
				nullptr, 0,
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
				&m_pAtlasBitmap
			));
		}
		return m_pAtlasBitmap;
	}

	const CRectU FindRect(const CSizeU& size)
	{
		auto sizeable = [](const CRectU& rc, const CSizeU& sz)->bool
		{
			return sz.width <= rc.Width() + 1 && sz.height <= rc.Height();
		};
		auto overlap = [](const CRectU& rc1, const CRectU& rc2)->bool
		{
			return (std::max)(rc1.left, rc2.left) < (std::min)(rc1.right, rc2.right) && (std::max)(rc1.top, rc2.top) < (std::min)(rc1.bottom, rc2.bottom);
		};
		auto subtract = [&overlap](const CRectU& l, const CRectU& r)->std::vector<CRectU>
		{
			std::vector<CRectU> rects;
			if (overlap(l, r)) {
				//if ((l.left < r.left && r.left < l.right) && (std::max)(l.top, r.top) < (std::min)(l.bottom, r.bottom))
				//	rects.emplace_back(l.left, l.top, l.left + r.left - l.left, l.top + l.Height());

				//if ((l.left < r.right && r.right < l.right) && (std::max)(l.top, r.top) < (std::min)(l.bottom, r.bottom))
				//	rects.emplace_back(r.right, l.top, r.right + l.right - r.right, l.top + l.Height());

				//if ((l.top < r.top && r.top < l.bottom) && (std::max)(l.left, r.left) < (std::min)(l.right, r.right))
				//	rects.emplace_back(l.left, l.top, l.left + l.Width(), l.top + r.top - l.top);

				//if ((l.top < r.bottom && r.bottom < l.bottom) && (std::max)(l.left, r.left) < (std::min)(l.right, r.right))
				//	rects.emplace_back(l.left, r.bottom, l.left + l.Width(), r.bottom + l.bottom - r.bottom);
				if ((l.left < r.left && r.left < l.right) && (std::max)(l.top, r.top) < (std::min)(l.bottom, r.bottom))
					rects.emplace_back(l.left, l.top, r.left - 1, l.bottom);

				if ((l.left < r.right && r.right < l.right) && (std::max)(l.top, r.top) < (std::min)(l.bottom, r.bottom))
					rects.emplace_back(r.right + 1, l.top, l.right, l.bottom);

				if ((l.top < r.top && r.top < l.bottom) && (std::max)(l.left, r.left) < (std::min)(l.right, r.right))
					rects.emplace_back(l.left, l.top, l.right, r.top - 1);

				if ((l.top < r.bottom && r.bottom < l.bottom) && (std::max)(l.left, r.left) < (std::min)(l.right, r.right))
					rects.emplace_back(l.left, r.bottom + 1, l.right, l.bottom);
			} else {
				rects.push_back(l);
			}
			return rects;
		};

		auto comp = [](const CRectU& l, const CRectU& r)->bool
		{
			const auto topCmp = l.top <=> r.top;
			if (topCmp == std::strong_ordering::less) { return true; }
			if (topCmp == std::strong_ordering::greater) { return false; }
			const auto leftCmp = l.left <=> r.left;
			if (leftCmp == std::strong_ordering::less) { return true; }
			if (leftCmp == std::strong_ordering::greater) { return false; }
			return false;
		};
		//New Rect
		CRectU newRect;
		for (const CRectU& room : m_rooms) {
			if (sizeable(room, size)) {
				newRect.SetRect(room.left, room.top, room.left + size.width - 1, room.top + size.height - 1);
				break;
			}
		}
		::OutputDebugString(std::format(L"NewRect: {}, {}, {}, {}\r\n", newRect.left, newRect.top, newRect.right, newRect.bottom).c_str());
		if (newRect.IsRectNull()) { return newRect; }

		//Subtract
		std::vector<CRectU> newRooms;
		for (const CRectU& room : m_rooms) {
			std::vector<CRectU> subtractRooms(subtract(room, newRect));
			newRooms.insert(newRooms.cend(), subtractRooms.cbegin(), subtractRooms.cend());
		}
		for (std::size_t i = 0; i < newRooms.size(); i++) {
			::OutputDebugString(std::format(L"NewRooms[{}]: {}, {}, {}, {}\r\n", i, newRooms[i].left, newRooms[i].top, newRooms[i].right, newRooms[i].bottom).c_str());
		}
		//Remove Rect in Rect
		m_rooms.clear();
		std::copy_if(newRooms.cbegin(), newRooms.cend(), std::back_inserter(m_rooms), [newRooms](const CRectU& l)->bool
			{
				return !std::any_of(newRooms.cbegin(), newRooms.cend(), [l](const CRectU& r)->bool { return l != r && r.RectInRect(l); });
			});

		std::sort(m_rooms.begin(), m_rooms.end(), comp);
		for (std::size_t i = 0; i < m_rooms.size(); i++) {
			::OutputDebugString(std::format(L"Rooms[{}]: {}, {}, {}, {}\r\n", i, m_rooms[i].left, m_rooms[i].top, m_rooms[i].right, m_rooms[i].bottom).c_str());
		}
		return newRect;
	}
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

