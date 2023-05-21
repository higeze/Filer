#include "D2DPDFBitmapDrawer.h"
#include "PDFPage.h"
#include "ThreadPool.h"
#include "D2DWTypes.h"


/***********************/
/* CD2DPDFBitmapDrawer */
/***********************/
CD2DPDFBitmapDrawer::CD2DPDFBitmapDrawer()
	:m_pAtlasFullOrClipBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
		CSizeU(4096, 8196),
		D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)))),
	m_pAtlasBlurBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
			CSizeU(2048, 4096),
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
		m_optPrimaryBitmapSize.emplace(m_pAtlasFullOrClipBitmap->GetSize());
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
			UHBITMAP phBmp = key.PagePtr->GetClipBitmap(pDirect->GetHDC(), key.Scale, key.Rotate, key.Rect);

			CComPtr<IWICBitmap> pWICBmp;
			FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapFromHBITMAP(phBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBmp));

			CComPtr<ID2D1Bitmap1> pD2DBmp;
			FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBmp, &pD2DBmp));

			m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, pD2DBmp);
			callback();
		} else {
			m_pAtlasFullOrClipBitmap->Erase(key);
		}
	};
	
	if (!m_pAtlasFullOrClipBitmap->Exist(key)) {
		::OutputDebugString(std::format(L"NOT EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
			(LONG)(key.PagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
		m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, nullptr);
		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(funadd, 0));
		return false;
	} else {
		::OutputDebugString(std::format(L"EXIST: Page:{}, Scale:{}, Rect:{},{},{},{}\r\n",
			(LONG)(key.PagePtr), key.Scale, key.Rect.left, key.Rect.top, key.Rect.right, key.Rect.bottom).c_str());
		return m_pAtlasFullOrClipBitmap->DrawBitmap(pDirect, key, dstPoint);
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
		UHBITMAP phBmp = key.PagePtr->GetBitmap(pDirect->GetHDC(), key.Scale, key.Rotate);

		CComPtr<IWICBitmap> pWICBmp;
		FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapFromHBITMAP(phBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBmp));

		CComPtr<ID2D1Bitmap1> pD2DBmp;
		FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBmp, &pD2DBmp));

		m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, pD2DBmp);
		callback();
	};
	
	if (!m_pAtlasFullOrClipBitmap->Exist(key)) {
		m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, nullptr);
		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(funadd, 0));
		return false;
	} else {
		return m_pAtlasFullOrClipBitmap->DrawBitmap(pDirect, key, dstPoint);
	}
}

bool CD2DPDFBitmapDrawer::DrawPDFPageBlurBitmap(
		const CDirect2DWrite* pDirect,
		const PdfBmpKey& key,
		const CRectF& dstRect,
		std::function<void()>&& callback)
{
	auto funadd = [pDirect, key, callback, this]()->void
	{
		UHBITMAP phBmp = key.PagePtr->GetBitmap(pDirect->GetHDC(), key.Scale, key.Rotate);

		CComPtr<IWICBitmap> pWICBmp;
		FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapFromHBITMAP(phBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBmp));

		CComPtr<ID2D1Bitmap1> pD2DBmp;
		FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBmp, &pD2DBmp));

		m_pAtlasBlurBitmap->AddOrAssign(pDirect, key, pD2DBmp);
		callback();
	};
	
	if (!m_pAtlasBlurBitmap->Exist(key)) {
		m_pAtlasBlurBitmap->AddOrAssign(pDirect, key, nullptr);
		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(funadd, 10));
		return false;
	} else {
		return m_pAtlasBlurBitmap->DrawBitmap(pDirect, key, dstRect);
	}
}

bool CD2DPDFBitmapDrawer::ExistInFullOrClipBitmap(const PdfBmpKey& key) const
{
	return m_pAtlasFullOrClipBitmap->Exist(key);
}

std::vector<PdfBmpKey> CD2DPDFBitmapDrawer::FindFulKeys(std::function<bool(const PdfBmpKey&)>&& pred)
{
	std::vector<PdfBmpKey> keys(m_pAtlasFullOrClipBitmap->Keys());
	std::vector<PdfBmpKey> findKeys;
	std::copy_if(keys.cbegin(), keys.cend(), std::back_inserter(findKeys), pred);
	return findKeys;
}

void CD2DPDFBitmapDrawer::Clear()
{
	m_pAtlasFullOrClipBitmap->Clear();
	m_pAtlasBlurBitmap->Clear();
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