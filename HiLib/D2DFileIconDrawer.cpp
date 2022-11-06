#include "D2DFileIconDrawer.h"

/************************/
/* CShellFileIconDrawer */
/************************/
CD2DFileIconDrawer::CD2DFileIconDrawer(CDirect2DWrite* pDirect)
	:m_pDirect(pDirect), m_pAtlasBitmap(std::make_unique<CD2DAtlasFixedSizeBitmap>(pDirect, CSizeU(256, 256), CSizeU(16, 16))),
		m_excludeExtSet({L".exe", L".ico", L".lnk", L"known", L"drive"}) {}

CIcon CD2DFileIconDrawer::GetDefaultIcon() const
{
	HINSTANCE hDll = ::LoadLibrary(L"SHELL32.dll");
	//return CIcon(::LoadIcon(hDll, MAKEINTRESOURCE(1)));
	return CIcon(HICON(::LoadImageW(hDll, MAKEINTRESOURCE(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR)));
}

CIcon CD2DFileIconDrawer::GetIcon(const CIDL& absoluteIDL) const
{
	SHFILEINFO sfi = { 0 };
	::SHGetFileInfo((LPCTSTR)absoluteIDL.ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	return CIcon(sfi.hIcon);
}

CComPtr<ID2D1Bitmap> CD2DFileIconDrawer::GetBitmapFromIcon(const CIcon& icon) const
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

	CComPtr<ID2D1Bitmap> pBitmap;
	HRESULT hr = m_pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICFormatConverter, nullptr, &pBitmap);
	if (FAILED(hr)) {
		throw std::exception(FILE_LINE_FUNC);
	}
	return pBitmap;
}

void CD2DFileIconDrawer::DrawFileIconBitmap(const CRectF& dstRect, const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, const DWORD& data, std::function<void()> funupd)
{
	std::wstring key = L"DEFAULT";
	if (!m_pAtlasBitmap->Exist(key)) {
		m_pAtlasBitmap->Add(key, GetBitmapFromIcon(GetDefaultIcon()));
	}

	auto funbmp = [absoluteIDL, this]()->CComPtr<ID2D1Bitmap>
	{
		return GetBitmapFromIcon(GetIcon(absoluteIDL));
	};

	if (!absoluteIDL || path.empty()) {
		//Default Icon
	} else if (!ext.empty() && m_excludeExtSet.find(ext) == m_excludeExtSet.end() && data != 0) {
		key = ext;

	} else {
		key = path;
	}

	if (!m_pAtlasBitmap->Exist(key)) {
		::OutputDebugString(std::format(L"{}\r\n", key).c_str());
		m_pAtlasBitmap->AddAsync(key, funbmp, std::forward<decltype(funupd)>(funupd), L"DEFAULT");
	}
	m_pAtlasBitmap->DrawBitmapByKey(key, dstRect);
}

void CD2DFileIconDrawer::Clear()
{
	m_pAtlasBitmap->Clear();
}