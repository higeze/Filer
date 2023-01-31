#include "D2DPDFBitmapDrawer.h"
#include "PDFPage.h"
#include "ThreadPool.h"


/************************/
/* CD2DPDFBitmapDrawer */
/************************/
CD2DPDFBitmapDrawer::CD2DPDFBitmapDrawer()
	:m_pAtlasClipBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
		CSizeU(4096, 4096),
		D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)))),
	m_pAtlasSmallBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
			CSizeU(1024, 1024),
			D2D1::BitmapProperties1(
						D2D1_BITMAP_OPTIONS_NONE,
						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)))){}

bool CD2DPDFBitmapDrawer::DrawPDFPageClipBitmap(
		const CDirect2DWrite* pDirect,
		const PdfBmpKey& key,
		const CRectF& dstRect,
		std::function<void()>&& callback)
{
	m_curClipKey.set(key);
	auto funadd = [pDirect, key, callback, this]()->void
	{
		CComPtr<ID2D1Bitmap1> pBitmap = key.PagePtr->GetClipBitmap(pDirect, key.Scale, key.Rotate, key.Rect);
		//if (key == m_curClipKey.get()) {
			m_pAtlasClipBitmap->AddOrAssign(pDirect, key, pBitmap);
			callback();
		//}
	};
	
	
	if (!m_pAtlasClipBitmap->Exist(key)) {
		::OutputDebugString(std::format(L"NOT EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
			(LONG)(key.PagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
		m_pAtlasClipBitmap->AddOrAssign(pDirect, key, nullptr);
		CThreadPool::GetInstance()->enqueue(funadd);
		return false;
	} else {
		::OutputDebugString(std::format(L"EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
			(LONG)(key.PagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
		return m_pAtlasClipBitmap->DrawBitmap(pDirect, key, dstRect.LeftTop());
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
		CThreadPool::GetInstance()->enqueue(funadd);
		return false;
	} else {
		return m_pAtlasSmallBitmap->DrawBitmap(pDirect, key, dstRect);
	}
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
}