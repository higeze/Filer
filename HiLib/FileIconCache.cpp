#include "FileIconCache.h"
#include "MyIcon.h"
#include "MyCom.h"
#include "ThreadPool.h"

using namespace d2dw;

CFileIconCache::CFileIconCache(CDirect2DWrite* pDirect)
	:m_pDirect(pDirect), m_excludeExtSet({L".exe", L".ico", L".lnk", L"known", L"drive"}){}

CIcon CFileIconCache::GetIcon(const CIDL& absoluteIDL)
{
	SHFILEINFO sfi = { 0 };
	::SHGetFileInfo((LPCTSTR)absoluteIDL.ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	return CIcon(sfi.hIcon);
}

CComPtr<ID2D1Bitmap> CFileIconCache::GetBitmapFromIcon(const CIcon& icon)
{
	CComPtr<IWICBitmap> pWICBitmap;
	if (FAILED(m_pDirect->GetWICImagingFactory()->CreateBitmapFromHICON(icon, &pWICBitmap))) {
		throw std::exception(FILELINEFUNCTION);
	}
	CComPtr<IWICFormatConverter> pWICFormatConverter;
	if (FAILED(m_pDirect->GetWICImagingFactory()->CreateFormatConverter(&pWICFormatConverter))) {
		throw std::exception(FILELINEFUNCTION);
	}
	if (FAILED(pWICFormatConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) {
		throw std::exception(FILELINEFUNCTION);
	}
	double dpix = 96.0f, dpiy = 96.0f;
	if (FAILED(pWICFormatConverter->GetResolution(&dpix, &dpiy))) {
		throw std::exception(FILELINEFUNCTION);
	}

	D2D1_BITMAP_PROPERTIES bitmapProps;
	//bitmapProps.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
	bitmapProps.dpiX = (FLOAT)dpix;
	bitmapProps.dpiY = (FLOAT)dpiy;
	bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	//bitmapProps.colorContext = nullptr;

	CComPtr<ID2D1Bitmap> pBitmap;
	HRESULT hr = m_pDirect->GetHwndRenderTarget()->CreateBitmapFromWicBitmap(pWICBitmap, bitmapProps, &pBitmap);
	if (FAILED(hr)) {
		throw std::exception(FILELINEFUNCTION);
	}
	return pBitmap;
}

CComPtr<ID2D1Bitmap> CFileIconCache::GetFileIconBitmap(const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, std::function<void()> updated)
{ 
	if (!ext.empty() && m_excludeExtSet.find(ext) == m_excludeExtSet.end()) {
		if (auto iter = m_extMap.lock_find(ext); iter != m_extMap.end()) {
			return iter->second;
		} else {
			m_extMap.lock_emplace(std::make_pair(ext, GetDefaultIconBitmap()));
			CThreadPool::GetInstance()->enqueue([this](const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, std::function<void()> updated) {
				CCoInitializer coinit(COINIT_APARTMENTTHREADED);
				CComPtr<ID2D1Bitmap> pBitmap = GetBitmapFromIcon(GetIcon(absoluteIDL));
				m_extMap.lock_insert_or_assign(ext, std::move(pBitmap));
				if (updated) {
					updated();
				}
			}, absoluteIDL, path, ext, updated);
			return GetDefaultIconBitmap();
		}
	} else {
		if (auto iter = m_pathMap.lock_find(path); iter != m_pathMap.end()) {
			return iter->second;
		} else {
			m_pathMap.lock_emplace(std::make_pair(path, GetDefaultIconBitmap()));
			CThreadPool::GetInstance()->enqueue([this](const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, std::function<void()> updated) {
				CCoInitializer coinit(COINIT_APARTMENTTHREADED);
				CComPtr<ID2D1Bitmap> pBitmap = GetBitmapFromIcon(GetIcon(absoluteIDL));
				m_pathMap.lock_insert_or_assign(path, std::move(pBitmap));
				if (updated) {
					updated();
				}
			}, absoluteIDL, path, ext, updated);
			return GetDefaultIconBitmap();
		}
	}
}

//CComPtr<ID2D1Bitmap> CFileIconCache::GetDriveIconBitmap(const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext)
//{
//	if (auto iter = m_driveMap.lock_find(path); iter != m_driveMap.end()) {
//		return iter->second;
//	} else {
//		SHFILEINFO sfi = { 0 };
//		::SHGetFileInfo((LPCTSTR)absoluteIDL.ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
//		CIcon icon(sfi.hIcon);
//		CComPtr<ID2D1Bitmap> pBitmap = GetBitmapFromIcon(icon);
//		m_driveMap.lock_emplace(std::make_pair(path, pBitmap));
//		return pBitmap;
//	}
//}
//
//CComPtr<ID2D1Bitmap> CFileIconCache::GetKnownIconBitmap(const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext)
//{
//	if (auto iter = m_knownMap.lock_find(path); iter != m_knownMap.end()) {
//		return iter->second;
//	} else {
//		SHFILEINFO sfi = { 0 };
//		::SHGetFileInfo((LPCTSTR)absoluteIDL.ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
//		CIcon icon(sfi.hIcon);
//		CComPtr<ID2D1Bitmap> pBitmap = GetBitmapFromIcon(icon);
//		m_knownMap.lock_emplace(std::make_pair(path, pBitmap));
//		return pBitmap;
//	}
//}
//
CComPtr<ID2D1Bitmap> CFileIconCache::GetDefaultIconBitmap()
{
	if (!m_defaultIconBmp) {
		HINSTANCE hDll = ::LoadLibrary(L"SHELL32.dll");
		CIcon defaultIcon(::LoadIcon(hDll, MAKEINTRESOURCE(1)));
		m_defaultIconBmp = GetBitmapFromIcon(defaultIcon);
	}
	return m_defaultIconBmp;
}
//
//CComPtr<ID2D1Bitmap> CFileIconCache::GetFolderIconBitmap()
//{
//	if (!m_folderIconBmp) {
//		auto folder = CKnownFolderManager::GetInstance()->GetKnownFolderById(FOLDERID_ProgramFiles);
//		SHFILEINFO sfi = { 0 };
//		::SHGetFileInfo((LPCTSTR)(folder->GetAbsoluteIdl().ptr()), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
//		CIcon folderIcon(sfi.hIcon);
//		m_folderIconBmp = GetBitmapFromIcon(folderIcon);
//	}
//	return m_folderIconBmp;
//}
