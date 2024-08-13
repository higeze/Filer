#include "ImageDrawer.h"
#include "ThreadPool.h"
#include "D2DImage.h"
#include "D2DWTypes.h"

//void CImageDrawer::AddClipBitmap(
//	const CDirect2DWrite* pDirect,
//	const ImgBmpKey& key,
//	std::function<void()> callback)
//{
//	do {
//		FALSE_BREAK(key == m_curClipKey.get());
//
//		CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetClipBitmap(pDirect->GetD2DDeviceContext(), key.Scale, key.Rect, [this, key]()->bool { return key != m_curClipKey.get(); });
//		FALSE_BREAK(pBitmap);
//		m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, pBitmap);
//		callback();
//		return;
//
//	} while (1);
//
//	m_pAtlasPrimaryBitmap->Erase(key);
//}
//
//void CImageDrawer::AddBitmap(
//	const CDirect2DWrite* pDirect,
//	const ImgBmpKey& key,
//	std::function<void()> callback)
//{
//	do {
//		FALSE_BREAK(key == m_curKey.get());
//
//		CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetBitmap(pDirect->GetD2DDeviceContext(), key.Scale, [this, key]()->bool { return key != m_curKey.get(); });
//		FALSE_BREAK(pBitmap);
//		m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, pBitmap);
//
//		callback();
//		return;
//	} while (1);
//
//	m_pAtlasPrimaryBitmap->Erase(key);
//}

//void CImageDrawer::AddBlurBitmap(
//	const CDirect2DWrite* pDirect,
//	const ImgBmpKey& key,
//	std::function<void()> callback)
//{
//	do {
//		FALSE_BREAK(key == m_curBlurKey.get());
//
//		CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetBitmap(pDirect->GetD2DDeviceContext(), key.Scale);
//		FALSE_BREAK(pBitmap);
//		m_pAtlasSecondaryBitmap->AddOrAssign(pDirect, key, pBitmap);
//
//		callback();
//		return;
//	} while (1);
//
//	m_pAtlasSecondaryBitmap->Erase(key);
//}

//#include "ThreadPool.h"
//#include "D2DImage.h"
//#include "D2DWTypes.h"
//
//
///***********************/
///* CImageDrawer */
///***********************/
//CImageDrawer::CImageDrawer()
//	:m_pAtlasClipBitmap(std::make_unique<CD2DAtlasBitmap<ImgBmpKey>>(
//		CSizeU(4096, 8196),
//		D2D1::BitmapProperties1(
//					D2D1_BITMAP_OPTIONS_NONE,
//					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))),
//	m_pAtlasSmallBitmap(std::make_unique<CD2DAtlasBitmap<ImgBmpKey>>(
//			CSizeU(1024, 2024),
//			D2D1::BitmapProperties1(
//						D2D1_BITMAP_OPTIONS_NONE,
//						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))){}
//
//CImageDrawer::~CImageDrawer() = default;
//
//const std::unique_ptr<CThreadPool>& CImageDrawer::GetThreadPoolPtr() const
//{
//	if (!m_optThreadPoolPtr.has_value()) {
//		m_optThreadPoolPtr.emplace(std::make_unique<CThreadPool>(2));
//	}
//	return m_optThreadPoolPtr.value();
//}
//
//const CSizeU& CImageDrawer::GetPrimaryBitmapSize() const
//{
//	if (!m_optPrimaryBitmapSize.has_value()) {
//		m_optPrimaryBitmapSize.emplace(m_pAtlasClipBitmap->GetSize());
//	}
//	return m_optPrimaryBitmapSize.value();
//}
//
//bool CImageDrawer::DrawClipBitmap(
//		const CDirect2DWrite* pDirect,
//		const ImgBmpKey& key,
//		const CPointF& dstPoint,
//		std::function<void()>&& callback)
//{
//	m_curClipKey.set(key);
//	auto funadd = [pDirect, key, callback, this]()->void
//	{
//		CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetClipBitmap(pDirect->GetD2DDeviceContext(), key.Scale, key.Rect, [this, key]()->bool { return key != GetCurClipKey(); });
//		if (pBitmap) {
//			::OutputDebugString(std::format(L"ADDORASSIGN: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
//				(LONG)(key.ImagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
//			m_pAtlasClipBitmap->AddOrAssign(pDirect, key, pBitmap);
//			callback();
//		} else {
//			::OutputDebugString(std::format(L"ERASE: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
//				(LONG)(key.ImagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
//			m_pAtlasClipBitmap->Erase(key);
//		}
//
//		//if (key == m_curClipKey.get()){
//		//	CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetClipBitmap(pDirect->GetD2DDeviceContext(), key.Scale, key.Rect);
//		//	m_pAtlasClipBitmap->AddOrAssign(pDirect, key, pBitmap);
//		//	callback();
//		//} else {
//		//	m_pAtlasClipBitmap->Erase(key);
//		//}
//	};
//	
//	if (!m_pAtlasClipBitmap->Exist(key)) {
//		::OutputDebugString(std::format(L"NOT EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
//			(LONG)(key.ImagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
//		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, CComPtr<ID2D1Bitmap1>(nullptr));
//		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0, funadd));
//		return false;
//	} else {
//		::OutputDebugString(std::format(L"EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
//			(LONG)(key.ImagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
//		return m_pAtlasClipBitmap->DrawBitmap(pDirect, key, dstPoint);
//	}
//}
//
//bool CImageDrawer::DrawClipBitmap(
//		const CDirect2DWrite* pDirect,
//		const ImgBmpKey& key,
//		const CRectF& dstRect,
//		std::function<void()>&& callback)
//{
//	m_curClipKey.set(key);
//	auto funadd = [pDirect, key, callback, this]()->void
//	{
//		CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetClipBitmap(pDirect->GetD2DDeviceContext(), key.Scale, key.Rect, [this, key]()->bool { return key != GetCurClipKey(); });
//		if (pBitmap) {
//			m_pAtlasClipBitmap->AddOrAssign(pDirect, key, pBitmap);
//			callback();
//		} else {
//			m_pAtlasClipBitmap->Erase(key);
//		}
//	};
//	
//	if (!m_pAtlasClipBitmap->Exist(key)) {
//		::OutputDebugString(std::format(L"NOT EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
//			(LONG)(key.ImagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
//		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, CComPtr<ID2D1Bitmap1>(nullptr));
//		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0, funadd));
//		return false;
//	} else {
//		::OutputDebugString(std::format(L"EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
//			(LONG)(key.ImagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
//		return m_pAtlasClipBitmap->DrawBitmap(pDirect, key, dstRect);
//	}
//}
//
//bool CImageDrawer::DrawBitmap(
//		const CDirect2DWrite* pDirect,
//		const ImgBmpKey& key,
//		const CPointF& dstPoint,
//		std::function<void()>&& callback)
//{
//	auto funadd = [pDirect, key, callback, this]()->void
//	{
//		CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetBitmap(pDirect->GetD2DDeviceContext(), key.Scale);
//		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, pBitmap);
//		callback();
//	};
//	
//	if (!m_pAtlasClipBitmap->Exist(key)) {
//		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, CComPtr<ID2D1Bitmap1>(nullptr));
//		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0, funadd));
//		return false;
//	} else {
//		return m_pAtlasClipBitmap->DrawBitmap(pDirect, key, dstPoint);
//	}
//}
//
//bool CImageDrawer::DrawBitmap(
//		const CDirect2DWrite* pDirect,
//		const ImgBmpKey& key,
//		const CRectF& dstRect,
//		std::function<void()>&& callback)
//{
//	auto funadd = [pDirect, key, callback, this]()->void
//	{
//		CComPtr<ID2D1Bitmap1> pBitmap = key.ImagePtr->GetBitmap(pDirect->GetD2DDeviceContext(), key.Scale);
//		m_pAtlasSmallBitmap->AddOrAssign(pDirect, key, pBitmap);
//		callback();
//	};
//	
//	if (!m_pAtlasSmallBitmap->Exist(key)) {
//		m_pAtlasSmallBitmap->AddOrAssign(pDirect, key, CComPtr<ID2D1Bitmap1>(nullptr));
//		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 10, funadd));
//		return false;
//	} else {
//		return m_pAtlasSmallBitmap->DrawBitmap(pDirect, key, dstRect);
//	}
//}
//
//bool CImageDrawer::ExistInPrimary(const ImgBmpKey& key) const
//{
//	return m_pAtlasClipBitmap->Exist(key);
//}
//
//std::vector<ImgBmpKey> CImageDrawer::FindClipKeys(std::function<bool(const ImgBmpKey&)>&& pred)
//{
//	std::vector<ImgBmpKey> keys(m_pAtlasClipBitmap->Keys());
//	std::vector<ImgBmpKey> findKeys;
//	std::copy_if(keys.cbegin(), keys.cend(), std::back_inserter(findKeys), pred);
//	return findKeys;
//}
//
//void CImageDrawer::Clear()
//{
//	m_pAtlasClipBitmap->Clear();
//	m_pAtlasSmallBitmap->Clear();
//	m_futureGroup.clean_up();
//}
//
//void CImageDrawer::WaitAll()
//{
//	m_futureGroup.wait_all();
//}