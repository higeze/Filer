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

class CIndexRect
{
public:
	UINT32 left = 0;
	UINT32 top = 0;
	UINT32 right = 0;
	UINT32 bottom = 0;

	CIndexRect(UINT32 l = 0, UINT32 t = 0, UINT32 r = 0, UINT32 b = 0)
		:left(l), top(t), right(r),bottom(b) {}
	void SetRect(UINT32 l, UINT32 t, UINT32 r, UINT32 b) 
	{
		left = l; top = t; right = r; bottom = b;
	}
	bool IsInvalid() const
	{
		return this->left >= this->right || this->top >= this->bottom;
	}
	bool SizeInRect(const CSizeU& sz) const
	{
		return sz.width <= this->Width() && sz.height <= this->Height();
	}
	bool RectInRect(const CIndexRect& rc) const
	{ 
		return rc.left >= this->left && rc.top >= this->top && rc.right <= this->right && rc.bottom <= this->bottom;
	}

	bool Overlap(const CIndexRect& rc) const
	{
		return (std::max)(this->left, rc.left) < (std::min)(this->right, rc.right) && (std::max)(this->top, rc.top) < (std::min)(this->bottom, rc.bottom);
	}
	bool TouchOrOverlap(const CIndexRect& rc) const
	{
		return (std::max)(this->left-1, rc.left-1) <= (std::min)(this->right, rc.right) && (std::max)(this->top-1, rc.top-1) <= (std::min)(this->bottom, rc.bottom);
	}

	std::vector<CIndexRect> Subtract(const CIndexRect& rc) const
	{
		std::vector<CIndexRect> rects;
		if (Overlap(rc)) {
			//if ((l.left < r.left && r.left < l.right) && (std::max)(l.top, r.top) < (std::min)(l.bottom, r.bottom))
			//	rects.emplace_back(l.left, l.top, l.left + r.left - l.left, l.top + l.Height());

			//if ((l.left < r.right && r.right < l.right) && (std::max)(l.top, r.top) < (std::min)(l.bottom, r.bottom))
			//	rects.emplace_back(r.right, l.top, r.right + l.right - r.right, l.top + l.Height());

			//if ((l.top < r.top && r.top < l.bottom) && (std::max)(l.left, r.left) < (std::min)(l.right, r.right))
			//	rects.emplace_back(l.left, l.top, l.left + l.Width(), l.top + r.top - l.top);

			//if ((l.top < r.bottom && r.bottom < l.bottom) && (std::max)(l.left, r.left) < (std::min)(l.right, r.right))
			//	rects.emplace_back(l.left, r.bottom, l.left + l.Width(), r.bottom + l.bottom - r.bottom);
			if ((this->left < rc.left && rc.left < this->right) && (std::max)(this->top, rc.top) < (std::min)(this->bottom, rc.bottom))
				rects.emplace_back(this->left, this->top, rc.left - 1, this->bottom);

			if ((this->left < rc.right && rc.right < this->right) && (std::max)(this->top, rc.top) < (std::min)(this->bottom, rc.bottom))
				rects.emplace_back(rc.right + 1, this->top, this->right, this->bottom);

			if ((this->top < rc.top && rc.top < this->bottom) && (std::max)(this->left, rc.left) < (std::min)(this->right, rc.right))
				rects.emplace_back(this->left, this->top, this->right, rc.top - 1);

			if ((this->top < rc.bottom && rc.bottom < this->bottom) && (std::max)(this->left, rc.left) < (std::min)(this->right, rc.right))
				rects.emplace_back(this->left, rc.bottom + 1, this->right, this->bottom);
		} else {
			rects.push_back(*this);
		}
		return rects;
	}
	std::vector<CIndexRect> Add(const CIndexRect& rc) const
	{
		std::vector<CIndexRect> tempRects;
		tempRects.push_back(*this);
		tempRects.push_back(rc);
		if (TouchOrOverlap(rc)) {
			tempRects.emplace_back(
				(std::min)(this->left, rc.left),
				(std::max)(this->top, rc.top),
				(std::max)(this->right, rc.right),
				(std::min)(this->bottom, rc.bottom)
			);
			tempRects.emplace_back(
				(std::max)(this->left, rc.left),
				(std::min)(this->top, rc.top),
				(std::min)(this->right, rc.right),
				(std::max)(this->bottom, rc.bottom)
			);
		}
		std::vector<CIndexRect> rects;
		std::copy_if(tempRects.cbegin(), tempRects.cend(), std::back_inserter(rects), [&tempRects](const CIndexRect& rc) {
			return !rc.IsInvalid() &&
				std::any_of(tempRects.cbegin(), tempRects.cend(), [&rc](const CIndexRect& rect) { return rect.RectInRect(rc); });
		});
		return rects;
	}

	auto operator<=>(const CIndexRect& rc) const
	{
		const auto topCmp = this->top <=> rc.top;
		if (topCmp != std::strong_ordering::equal) {
			return topCmp;
		} else {
			const auto leftCmp = this->left <=> rc.left;
			return leftCmp;
		}
	}
	bool operator==(const CIndexRect& rc) const
	{
		return this->left == rc.left && this->top == rc.top && this->right == rc.right && this->bottom == rc.bottom;
	}
	bool IsRectNull() const { return left == 0 && top == 0 && right == 0 && bottom == 0; }
	UINT32 Width() const { return right - left + 1; }
	UINT32 Height() const { return bottom - top + 1; }


};

template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = std::allocator<std::pair<const _Kty, CIndexRect>>>
class CD2DAtlasBitmap
{
protected:
	using map_type = std::unordered_map<_Kty, CIndexRect, _Hasher, _Keyeq, _Alloc>;
	using que_type = std::vector<_Kty>;
	CSizeU m_size;
	D2D1_BITMAP_PROPERTIES1 m_bitmapProperties;
	map_type m_map;
	que_type m_que;
	CComPtr<ID2D1Bitmap1> m_pAtlasBitmap;
	std::vector<CIndexRect> m_rooms;
	std::shared_mutex m_mtx;

public:
	CD2DAtlasBitmap(
		const CSizeU& size = CSizeU(512, 512), 
		const D2D1_BITMAP_PROPERTIES1& bitmapProperties = D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))
		:m_size(size), m_bitmapProperties(bitmapProperties) { m_rooms.emplace_back(0, 0, m_size.width - 1, m_size.height - 1); }

	CSizeU GetSize() const { return m_size; }

	bool DrawBitmap(const CDirect2DWrite* pDirect, const _Kty& key, const CRectF& dstRect)
	{
		std::shared_lock<std::shared_mutex> lock(m_mtx);
		
		if (auto iter = m_map.find(key); iter != m_map.cend() && !(iter->second.IsRectNull())) {
			m_que.erase(std::remove(std::begin(m_que), std::end(m_que), key), std::cend(m_que));
			m_que.push_back(key);

			CRectF dstRc(
				std::round(dstRect.left),
				std::round(dstRect.top),
				std::round(dstRect.right),
				std::round(dstRect.bottom));
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
			return true;
		} else {
			return false;
		}
	}
	bool DrawBitmap(const CDirect2DWrite* pDirect, const _Kty& key, const CPointF& dstPoint)
	{
		std::shared_lock<std::shared_mutex> lock(m_mtx);
		
		if (auto iter = m_map.find(key); iter != m_map.cend() && !(iter->second.IsRectNull())) {
			m_que.erase(std::remove(std::begin(m_que), std::end(m_que), key), std::cend(m_que));
			m_que.push_back(key);
			CRectF dstRc(
				std::round(dstPoint.x),
				std::round(dstPoint.y),
				std::round(dstPoint.x + iter->second.Width()),
				std::round(dstPoint.y + iter->second.Height()));
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
			return true;
		} else {
			return false;
		}
	}

	void Erase(const _Kty& key)
	{
		if (auto iter = m_map.find(key); iter != m_map.cend()) {
			m_map.erase(key);
			m_que.erase(std::remove(std::begin(m_que), std::end(m_que), key), std::cend(m_que));
		}
	}

	//void OptimizeRooms()
	//{
	//	std::vector<CIndexRect> newRooms;
	//	for (const CIndexRect& thisRect : m_rooms) {
	//		//Check if this Rect can Combine
	//		//Check if this Rect in Rects
	//		bool isThisRectInRects = std::any_of(m_rooms.cbegin(), m_rooms.cend(), [&thisRect](const CIndexRect& rect) {return rect.RectInRect(thisRect)});
	//	}
	//}

	//void PopRect()
	//{
	//	CIndexRect rcAdd(m_map[m_que.front()]);
	//	std::vector<CIndexRect> newRooms;
	//	for (const CIndexRect& rc : m_rooms) {
	//		std::vector<CIndexRect> addRooms(rcAdd.Add(rc));
	//		newRooms.insert(newRooms.end(), addRooms.cbegin(), addRooms.cend());
	//	}
	//	m_rooms.clear();
	//	std::copy_if(newRooms.cbegin(), newRooms.cend(), std::back_inserter(m_rooms), [newRooms](const CIndexRect& l)->bool
	//		{
	//			return !std::any_of(newRooms.cbegin(), newRooms.cend(), [l](const CIndexRect& r)->bool { return l != r && r.RectInRect(l); });
	//		});

	//	m_map.erase(m_que.front());
	//	m_que.erase(std::remove(std::begin(m_que), std::end(m_que), m_que.front()), std::cend(m_que));
	//}

	void AddOrAssign(const CDirect2DWrite* pDirect, const _Kty& key, const CComPtr<ID2D1Bitmap1>& pBitmap)
	{
		std::lock_guard<std::shared_mutex> lock(m_mtx);
		auto iter = m_map.find(key);
		if(pBitmap){
			CIndexRect newRect;
			CSizeU size(pBitmap->GetPixelSize());
			//Check size
			if (size.width > m_size.width || size.height > m_size.height) {
				return;
			}
			if (iter == m_map.cend() || iter->second.IsRectNull()) {
				newRect = FindRect(size);
				//while (newRect.IsRectNull()) {
				//	PopRect();
				//	newRect = FindRect(size);
				//}
				//pDirect->SaveBitmap(L"Test.png", GetAtlasBitmapPtr(pDirect));
				if (newRect.IsRectNull()) {
#ifdef _DEBUG
					pDirect->SaveBitmap(L"Test.png", GetAtlasBitmapPtr(pDirect));
#endif
					//m_pAtlasBitmap.Release();
					m_map.clear();
					m_que.clear();
					m_rooms.clear();
					m_rooms.emplace_back(0, 0, m_size.width - 1, m_size.height - 1);
					newRect = FindRect(size);
				}
			} else {
				newRect = iter->second;
			}
			D2D1_POINT_2U dstLeftTop{ newRect.left, newRect.top };
			D2D1_RECT_U srcRect{ 0, 0, size.width, size.height };

			FAILED_THROW(GetAtlasBitmapPtr(pDirect)->CopyFromBitmap(&dstLeftTop, pBitmap, &srcRect));
			m_map.insert_or_assign(key, newRect);
			m_que.push_back(key);
		} else {
			m_map.insert_or_assign(key, CIndexRect());
			m_que.push_back(key);
		}
	}

	std::vector<_Kty> Keys()
	{
		std::shared_lock<std::shared_mutex> lock(m_mtx);
		std::vector<_Kty> keys;
		std::transform(m_map.cbegin(), m_map.cend(), std::back_inserter(keys), [](const std::pair<const _Kty, CIndexRect>& pair) { return pair.first; });
		return keys;
	}

	bool Exist(const _Kty& key)
	{
		std::shared_lock<std::shared_mutex> lock(m_mtx);
		return m_map.find(key) != m_map.end();
	}

	//bool ExistAndAvailable(const _Kty& key)
	//{
	//	std::shared_lock<std::shared_mutex> lock(m_mtx);
	//	auto iter = m_map.find(key);
	//	return iter!= m_map.end() && !iter->second.IsRectNull()
	//}

	void Clear()
	{
		std::lock_guard<std::shared_mutex> lock(m_mtx);
		m_pAtlasBitmap.Release();
		m_map.clear();
		m_que.clear();
		m_rooms.clear();
		m_rooms.emplace_back(0, 0, m_size.width - 1, m_size.height - 1);
	}

private:	 
	const CComPtr<ID2D1Bitmap1>& GetAtlasBitmapPtr(const CDirect2DWrite* pDirect)
	{
		if (!m_pAtlasBitmap) {
			FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmap(
				m_size,
				nullptr, 0,
				m_bitmapProperties,
				&m_pAtlasBitmap
			));
		}
		return m_pAtlasBitmap;
	}

	const CIndexRect FindRect(const CSizeU& size)
	{
		CIndexRect newRect;

		//Check size
		if (size.width > m_size.width || size.height > m_size.height) {
			return newRect;
		}
		//New Rect
		for (const CIndexRect& room : m_rooms) {
			if (room.SizeInRect(size)) {
				newRect.SetRect(room.left, room.top, room.left + size.width - 1, room.top + size.height - 1);
				break;
			}
		}
		::OutputDebugString(std::format(L"NewRect: {}, {}, {}, {}\r\n", newRect.left, newRect.top, newRect.right, newRect.bottom).c_str());
		if (newRect.IsRectNull()) { return newRect; }

		//Subtract
		std::vector<CIndexRect> newRooms;
		for (const CIndexRect& room : m_rooms) {
			std::vector<CIndexRect> subtractRooms(room.Subtract(newRect));
			newRooms.insert(newRooms.cend(), subtractRooms.cbegin(), subtractRooms.cend());
		}
		for (std::size_t i = 0; i < newRooms.size(); i++) {
			::OutputDebugString(std::format(L"NewRooms[{}]: {}, {}, {}, {}\r\n", i, newRooms[i].left, newRooms[i].top, newRooms[i].right, newRooms[i].bottom).c_str());
		}
		//Remove Rect in Rect
		m_rooms.clear();
		std::copy_if(newRooms.cbegin(), newRooms.cend(), std::back_inserter(m_rooms), [newRooms](const CIndexRect& l)->bool
			{
				return !std::any_of(newRooms.cbegin(), newRooms.cend(), [l](const CIndexRect& r)->bool { return l != r && r.RectInRect(l); });
			});

		std::sort(m_rooms.begin(), m_rooms.end());
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

