#include "D2DPDFBitmapDrawer.h"
#include "PDFPage.h"
#include "ThreadPool.h"
#include "D2DWTypes.h"


/***********************/
/* CD2DPDFBitmapDrawer */
/***********************/
CD2DPDFBitmapDrawer::CD2DPDFBitmapDrawer()
	:m_pAtlasClipBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
		CSizeU(4096, 8196),
		D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)))),
	m_pAtlasSmallBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
			CSizeU(1024, 2024),
			D2D1::BitmapProperties1(
						D2D1_BITMAP_OPTIONS_NONE,
						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)))){}

CD2DPDFBitmapDrawer::~CD2DPDFBitmapDrawer() = default;

const std::unique_ptr<CThreadPool>& CD2DPDFBitmapDrawer::GetThreadPoolPtr() const
{
	if (!m_optThreadPoolPtr.has_value()) {
		m_optThreadPoolPtr.emplace(std::make_unique<CThreadPool>(4));
	}
	return m_optThreadPoolPtr.value();
}

const CSizeU& CD2DPDFBitmapDrawer::GetPrimaryBitmapSize() const
{
	if (!m_optPrimaryBitmapSize.has_value()) {
		m_optPrimaryBitmapSize.emplace(m_pAtlasClipBitmap->GetSize());
	}
	return m_optPrimaryBitmapSize.value();
}

//bool CD2DPDFBitmapDrawer::DrawPDFPageBitmap(
//	const CDirect2DWrite* pDirect,
//	const PdfBmpKey& key,
//	const CRectF& dstRect,
//	std::function<void()>&& callback)
//{
//
//}

//bool CD2DPDFBitmapDrawer::DrawPDFPage(
//	const CDirect2DWrite* pDirect,
//	const PdfBmpKey& key,
//	const CRectF& dstRect,
//	std::function<void()>&& callback)
//{
//	CRectF scaledRect = key.Rect * key.Scale;
//	bool drawFullPage = (scaledRect.Width() * scaledRect.Height()) < (m_pAtlasClipBitmap->GetSize().width * m_pAtlasClipBitmap->GetSize().height / 8);
//
//	if (drawFullPage) {
//		return DrawPDFPageBitmap(pDirect, key, dstRect.LeftTop(), std::forward<std::function<void()>>(callback));
//	} else {
//		if (m_pAtlasClipBitmap->Exist(key)) {
//			return DrawPDFPageClipBitmap(pDirect, key, dstRect.LeftTop(), std::forward<std::function<void()>>(callback));
//		} else {
//			PdfBmpKey smallKey(key);
//			smallKey.Scale = 0.2f;
//			DrawPDFPageBitmap(pDirect, smallKey, dstRect, std::forward<std::function<void()>>(callback));
//			return DrawPDFPageClipBitmap(pDirect, key, dstRect.LeftTop(), std::forward<std::function<void()>>(callback));
//		}
//	}
//}

bool CD2DPDFBitmapDrawer::DrawPDFPageClipBitmap(
		const CDirect2DWrite* pDirect,
		const PdfBmpKey& key,
		const CPointF& dstPoint,
		std::function<void()>&& callback)
{
	m_curClipKey.set(key);
	auto funadd = [pDirect, key, callback, this]()->void
	{
		if (key == m_curClipKey.get()){
			CComPtr<ID2D1Bitmap1> pBitmap = key.PagePtr->GetClipBitmap(pDirect, key.Scale, key.Rotate, key.Rect);
			m_pAtlasClipBitmap->AddOrAssign(pDirect, key, pBitmap);
			callback();
		} else {
			m_pAtlasClipBitmap->Erase(key);
		}
	};
	
	if (!m_pAtlasClipBitmap->Exist(key)) {
		::OutputDebugString(std::format(L"NOT EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
			(LONG)(key.PagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, nullptr);
		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(funadd, 0));
		return false;
	} else {
		::OutputDebugString(std::format(L"EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
			(LONG)(key.PagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
		return m_pAtlasClipBitmap->DrawBitmap(pDirect, key, dstPoint);
	}
}

bool CD2DPDFBitmapDrawer::DrawPDFPageBitmap(
		const CDirect2DWrite* pDirect,
		const PdfBmpKey& key,
		const CPointF& dstPoint,
		std::function<void()>&& callback)
{
	auto funadd = [pDirect, key, callback, this]()->void
	{
		CComPtr<ID2D1Bitmap1> pBitmap = key.PagePtr->GetBitmap(pDirect, key.Scale, key.Rotate);
		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, pBitmap);
		callback();
	};
	
	if (!m_pAtlasClipBitmap->Exist(key)) {
		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, nullptr);
		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(funadd, 0));
		return false;
	} else {
		return m_pAtlasClipBitmap->DrawBitmap(pDirect, key, dstPoint);
	}
}

bool CD2DPDFBitmapDrawer::DrawPDFPageBitmap(
		const CDirect2DWrite* pDirect,
		const PdfBmpKey& key,
		const CRectF& dstRect,
		std::function<void()>&& callback)
{
	auto funadd = [pDirect, key, callback, this]()->void
	{
		CComPtr<ID2D1Bitmap1> pBitmap = key.PagePtr->GetBitmap(pDirect, key.Scale, key.Rotate);
		m_pAtlasSmallBitmap->AddOrAssign(pDirect, key, pBitmap);
		callback();
	};
	
	if (!m_pAtlasSmallBitmap->Exist(key)) {
		m_pAtlasSmallBitmap->AddOrAssign(pDirect, key, nullptr);
		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(funadd, 10));
		return false;
	} else {
		return m_pAtlasSmallBitmap->DrawBitmap(pDirect, key, dstRect);
	}
}

bool CD2DPDFBitmapDrawer::ExistInPrimary(const PdfBmpKey& key) const
{
	return m_pAtlasClipBitmap->Exist(key);
}

std::vector<PdfBmpKey> CD2DPDFBitmapDrawer::FindClipKeys(std::function<bool(const PdfBmpKey&)>&& pred)
{
	std::vector<PdfBmpKey> keys(m_pAtlasClipBitmap->Keys());
	std::vector<PdfBmpKey> findKeys;
	std::copy_if(keys.cbegin(), keys.cend(), std::back_inserter(findKeys), pred);
	return findKeys;
}

void CD2DPDFBitmapDrawer::Clear()
{
	m_pAtlasClipBitmap->Clear();
	m_pAtlasSmallBitmap->Clear();
	m_futureGroup.clean_up();
}

//void CD2DPDFBitmapDrawer::CleanFutures()
//{
//	m_futureGroup.clean_up();
//}

void CD2DPDFBitmapDrawer::WaitAll()
{
	m_futureGroup.wait_all();
}