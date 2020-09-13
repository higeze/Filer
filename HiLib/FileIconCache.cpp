#include "FileIconCache.h"
#include "MyIcon.h"
#include "MyCom.h"
#include "ThreadPool.h"
#include "Debug.h"
#include <VersionHelpers.h>
#include <ntddkbd.h>


BOOL GetOSVersion(OSVERSIONINFOEX* os)
{
	HMODULE hMod;
	typedef void (WINAPI *RtlGetVersion_FUNC)(OSVERSIONINFOEX*);
	RtlGetVersion_FUNC func;

	hMod = LoadLibrary(TEXT("ntdll.dll"));
	if (hMod) {
		func = (RtlGetVersion_FUNC)GetProcAddress(hMod, "RtlGetVersion");
		if (func == 0) {
			FreeLibrary(hMod);
			return FALSE;
		}
		ZeroMemory(os, sizeof(*os));
		os->dwOSVersionInfoSize = sizeof(*os);
		func(os);

	} else
		return FALSE;
	FreeLibrary(hMod);
	return TRUE;
}


CFileIconCache::CFileIconCache(CDirect2DWrite* pDirect)
	:m_pDirect(pDirect), m_excludeExtSet({L".exe", L".ico", L".lnk", L"known", L"drive"})
{
	//OSVERSIONINFOEX os;
	//::GetOSVersion(&os);
	//m_isWin10 = os.dwMajorVersion == 10;
}

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
		throw std::exception(FILE_LINE_FUNC);
	}
	CComPtr<IWICFormatConverter> pWICFormatConverter;
	if (FAILED(m_pDirect->GetWICImagingFactory()->CreateFormatConverter(&pWICFormatConverter))) {
		throw std::exception(FILE_LINE_FUNC);
	}
	if (FAILED(pWICFormatConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) {
		throw std::exception(FILE_LINE_FUNC);
	}
	//double dpix = 96.0f, dpiy = 96.0f;
	//if (FAILED(pWICFormatConverter->GetResolution(&dpix, &dpiy))) {
	//	throw std::exception(FILE_LINE_FUNC);
	//}

	//D2D1_BITMAP_PROPERTIES bitmapProps;
	////bitmapProps.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
	//bitmapProps.dpiX = (FLOAT)dpix;
	//bitmapProps.dpiY = (FLOAT)dpiy;
	//bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	////bitmapProps.colorContext = nullptr;

	CComPtr<ID2D1Bitmap> pBitmap;
//	HRESULT hr = m_pDirect->GetHwndRenderTarget()->CreateBitmapFromWicBitmap(pWICBitmap, bitmapProps, &pBitmap);
	HRESULT hr = m_pDirect->GetHwndRenderTarget()->CreateBitmapFromWicBitmap(pWICFormatConverter, nullptr, &pBitmap);
	if (FAILED(hr)) {
		throw std::exception(FILE_LINE_FUNC);
	}
	return pBitmap;
}

CComPtr<ID2D1Bitmap> CFileIconCache::GetFileIconBitmap(const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, std::function<void()> updated)
{
	if (!absoluteIDL || path.empty()) {
		return GetDefaultIconBitmap();
	} else if (!ext.empty() && m_excludeExtSet.find(ext) == m_excludeExtSet.end()) {
		if (auto iter = m_extMap.lock_find(ext); iter != m_extMap.end()) {
			return iter->second;
		} else {
			m_extMap.lock_emplace(std::make_pair(ext, GetDefaultIconBitmap()));
			CThreadPool::GetInstance()->enqueue([this](const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, std::function<void()> updated) {
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

void CFileIconCache::Clear()
{
	m_extMap.clear();
	m_pathMap.clear();
	m_defaultIconBmp.Release();
}

CComPtr<ID2D1Bitmap> CFileIconCache::GetDefaultIconBitmap()
{
	if (!m_defaultIconBmp) {
		HINSTANCE hDll = ::LoadLibrary(L"SHELL32.dll");
		CIcon defaultIcon(::LoadIcon(hDll, MAKEINTRESOURCE(1)));
		m_defaultIconBmp = GetBitmapFromIcon(defaultIcon);
	}
	return m_defaultIconBmp;
}
