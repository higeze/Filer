#include "D2DThumbnailDrawer.h"
#include "WICImagingFactory.h"
#include <thumbcache.h>
#include <ShlGuid.h>
#include "Debug.h"
#include "MyUniqueHandle.h"
#include "future_then.h"
#include "ThreadPool.h"


/********************/
/* CThumbnailDrawer */
/********************/
CD2DThumbnailDrawer::CD2DThumbnailDrawer()
	:m_pAtlasBitmap(std::make_unique<CD2DAtlasBitmap<ThumbnailBmpKey>>(
	CSizeU(1024, 1024),
	D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_NONE,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))){}

CComPtr<IWICBitmapSource> CD2DThumbnailDrawer::GetThumbnailBitmapByThumbnailProvider(
		const std::wstring& dispName, 
		const UINT32& thumbSize)
{
	CComPtr<IWICBitmap> pWICBitmap;
	do {
		CComPtr<IShellItem2> pShellItem;
		FAILED_BREAK(::SHCreateItemFromParsingName(dispName.c_str(), nullptr, IID_PPV_ARGS(&pShellItem)));
		CComPtr<IThumbnailProvider> pThumbProvider;
		FAILED_BREAK(pShellItem->BindToHandler(NULL, BHID_ThumbnailHandler, IID_PPV_ARGS(&pThumbProvider)));
		WTS_ALPHATYPE wtsAlpha;
		HBITMAP hBitmap;
		FAILED_BREAK(pThumbProvider->GetThumbnail(thumbSize, &hBitmap, &wtsAlpha));
		std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(hBitmap);
		FAILED_BREAK(CWICImagingFactory::GetInstance()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapUsePremultipliedAlpha  , &pWICBitmap));
	} while (false);

	return CComPtr<IWICBitmapSource>(pWICBitmap);
}

CComPtr<IWICBitmapSource> CD2DThumbnailDrawer::GetThumbnailBitmapByShellImageFactory(
		const std::wstring& dispName, 
		const CSize& thumbSize)
{
	CComPtr<IWICBitmap> pWICBitmap;
	try {
		do {
			CComPtr<IShellItemImageFactory> pImageFactory;
			FAILED_BREAK(::SHCreateItemFromParsingName(dispName.c_str(), NULL, IID_PPV_ARGS(&pImageFactory)));
			HBITMAP hBitmap;
			FAILED_BREAK(pImageFactory->GetImage(thumbSize, SIIGBF_RESIZETOFIT, &hBitmap));
			std::unique_ptr<std::remove_pointer_t<HBITMAP>, delete_object>  pBmp(hBitmap);
			FAILED_BREAK(CWICImagingFactory::GetInstance()->CreateBitmapFromHBITMAP(pBmp.get(), nullptr, WICBitmapUsePremultipliedAlpha, &pWICBitmap));
		} while (false);
	}
	catch (...) {
		pWICBitmap = nullptr;
	}
	return CComPtr<IWICBitmapSource>(pWICBitmap);
}

CComPtr<IWICBitmapSource> CD2DThumbnailDrawer::GetThumbnailBitmapByThumbnailCache(
	const std::wstring& dispName,
	const UINT32& thumbSize)
{
	CComPtr<IWICBitmap> pWICBitmap;
	do {
		CComPtr<IShellItem2> pShellItem;
		FAILED_BREAK(::SHCreateItemFromParsingName(dispName.c_str(), NULL, IID_PPV_ARGS(&pShellItem)));
		CComPtr<IThumbnailCache> pThumbCache;
		pThumbCache.CoCreateInstance(CLSID_LocalThumbnailCache, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER);
		CComPtr<ISharedBitmap> pSharedBitmap;
		FAILED_BREAK(pThumbCache->GetThumbnail(pShellItem, thumbSize, WTS_FORCEEXTRACTION | WTS_SCALETOREQUESTEDSIZE, &pSharedBitmap, NULL, NULL));
		HBITMAP hBitmap;
		FAILED_BREAK(pSharedBitmap->GetSharedBitmap(&hBitmap));//This HBitmap is in shared memory, no need to DeleteObject
		FAILED_BREAK(CWICImagingFactory::GetInstance()->CreateBitmapFromHBITMAP(hBitmap, nullptr, WICBitmapUsePremultipliedAlpha, &pWICBitmap));
	} while (false);

	return CComPtr<IWICBitmapSource>(pWICBitmap);
}

CComPtr<IWICBitmapSource> CD2DThumbnailDrawer::GetIconBitmap(
		const std::wstring& dispName)
{
	CComPtr<IWICFormatConverter> pWICFormatConverter;
	do {
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo(dispName.c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_ADDOVERLAYS);
		CIcon icon(sfi.hIcon);
		CComPtr<IWICBitmap> pWICBitmap;
		FAILED_BREAK(CWICImagingFactory::GetInstance()->CreateBitmapFromHICON(icon, &pWICBitmap));
		FAILED_BREAK(CWICImagingFactory::GetInstance()->CreateFormatConverter(&pWICFormatConverter));
		FAILED_BREAK(pWICFormatConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut));
	} while (false);
	return CComPtr<IWICBitmapSource>(pWICFormatConverter);
}

bool CD2DThumbnailDrawer::DrawThumbnailBitmap(
		const CDirect2DWrite* pDirect,
		const ThumbnailBmpKey& key,
		const CPointF& dstPoint)
{
	auto funadd = [pDirect, key, this]()->void
	{
		CComPtr<IWICBitmapSource> pBitmap = GetThumbnailBitmapByShellImageFactory(key.Name, CSize(key.Size, key.Size));
		m_pAtlasBitmap->AddOrAssign(pDirect, key, pBitmap);
	};
	
	if (!m_pAtlasBitmap->Exist(key)) {
		m_pAtlasBitmap->AddOrAssign(pDirect, key, CComPtr<IWICBitmapSource>(nullptr));
		m_futureGroup.emplace_back(CThreadPool::GetInstance()->enqueue(FILE_LINE_FUNC, 0, funadd));
		return false;
	} else {
		return m_pAtlasBitmap->DrawBitmap(pDirect, key, dstPoint);
	}
}

void CD2DThumbnailDrawer::Clear()
{
	m_pAtlasBitmap->Clear();
	m_futureGroup.clean_up();
}