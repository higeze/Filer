#include "D2DFileIconDrawer.h"
#include "WICImagingFactory.h"
#include "ThreadPool.h"
#include "ShellFile.h"

/************************/
/* CShellFileIconDrawer */
/************************/
CD2DFileIconDrawer::CD2DFileIconDrawer()
	:m_pAtlasBitmap(std::make_unique<CD2DAtlasBitmap<int>>(
		CSizeU(256, 256),
		D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))) {}

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

CComPtr<IWICBitmapSource> CD2DFileIconDrawer::GetBitmapSourceFromIcon(const CIcon& icon) const
{
	CComPtr<IWICBitmap> pWICBitmap;
	CWICImagingFactory::GetInstance()->CreateBitmapFromHICON(
		icon,
		&pWICBitmap
	);
	CComPtr<IWICFormatConverter> pWICFormatConverter;
	FAILED_THROW(CWICImagingFactory::GetInstance()->CreateFormatConverter(&pWICFormatConverter));
	FAILED_THROW(pWICFormatConverter->Initialize(
		pWICBitmap,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeMedianCut));
	return CComPtr<IWICBitmapSource>(pWICFormatConverter);
}

CComPtr<IWICBitmapSource> CD2DFileIconDrawer::GetBitmapSourceFromBitmap(const CBitmap& bmp) const
{
	CComPtr<IWICBitmap> pWICBitmap;
	CWICImagingFactory::GetInstance()->CreateBitmapFromHBITMAP(
		bmp,
		nullptr,
		WICBitmapUseAlpha,
		&pWICBitmap
	);
	CComPtr<IWICFormatConverter> pWICFormatConverter;
	FAILED_THROW(CWICImagingFactory::GetInstance()->CreateFormatConverter(&pWICFormatConverter));
	FAILED_THROW(pWICFormatConverter->Initialize(
		pWICBitmap,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeMedianCut));
	return CComPtr<IWICBitmapSource>(pWICFormatConverter);
}

//bool CD2DFileIconDrawer::DrawFileIconBitmap(
//	const CDirect2DWrite* pDirect,
//	const CPointF& dstPoint,
//	const CIDL& absoluteIDL,
//	const std::wstring& path,
//	const std::wstring& ext,
//	const DWORD& data,
//	std::function<void()>&& callback)
//{
//	std::wstring key = L"DEFAULT";
//	if (!m_pAtlasBitmap->Exist(key)) {
//		m_pAtlasBitmap->AddOrAssign(pDirect, key, GetBitmapSourceFromIcon(GetDefaultIcon()));
//	}
//
//	if (!absoluteIDL || path.empty()) {
//		key = L"DEFAULT";
//	} else if (!ext.empty() && m_excludeExtSet.find(ext) == m_excludeExtSet.end() && data != 0) {
//		key = ext;
//
//	} else {
//		key = path;
//	}
//	auto funadd = [pDirect, key, absoluteIDL, callback, this]()->void
//	{
//		CComPtr<IWICBitmapSource> pBitmap = GetBitmapFromIcon(GetIcon(absoluteIDL));
//		m_pAtlasBitmap->AddOrAssign(pDirect, key, pBitmap);
//		callback();
//	};
//	
//	bool ret = false;
//	if (!m_pAtlasBitmap->Exist(key)) {
//		m_pAtlasBitmap->AddOrAssign(pDirect, key, CComPtr<IWICBitmapSource>(nullptr));
//		m_futureGroup.emplace_back(CThreadPool::GetInstance()->enqueue(
//			FILE_LINE_FUNC,
//			0,
//			funadd));
//		ret =  false;
//	} else {
//		ret =  m_pAtlasBitmap->DrawBitmap(pDirect, key, dstPoint);
//	}
//
//	if (!ret) {
//		m_pAtlasBitmap->DrawBitmap(pDirect, L"DEFAULT", dstPoint);
//	}
//	return ret;
//
//
//	//auto funbmp = [absoluteIDL, this]()->CComPtr<ID2D1Bitmap>
//	//{
//	//	return GetBitmapFromIcon(GetIcon(absoluteIDL));
//	//};
//
//	//std::wstring key = L"DEFAULT";
//	//if (!m_pAtlasBitmap->Exist(key)) {
//	//	m_pAtlasBitmap->AddOrAssign(key, GetBitmapFromIcon(GetDefaultIcon()));
//	//}
//
//
//	//if (!absoluteIDL || path.empty()) {
//	//	//Default Icon
//	//} else if (!ext.empty() && m_excludeExtSet.find(ext) == m_excludeExtSet.end() && data != 0) {
//	//	key = ext;
//
//	//} else {
//	//	key = path;
//	//}
//
//	//if (!m_pAtlasBitmap->Exist(key)) {
//	//	//::OutputDebugString(std::format(L"{}\r\n", key).c_str());
//	//	m_pAtlasBitmap->AddAsync(key, funbmp, std::forward<decltype(funupd)>(funupd), L"DEFAULT");
//	//}
//	//m_pAtlasBitmap->DrawBitmapByKey(key, dstRect);
//}

bool CD2DFileIconDrawer::DrawDefaultIconBitmap(
	const CDirect2DWrite* pDirect,
	const CPointF& dstPoint)
{
	if (!m_pAtlasBitmap->Exist(INT_MAX)) {
		m_pAtlasBitmap->AddOrAssign(pDirect, INT_MAX, GetBitmapSourceFromIcon(GetDefaultIcon()));
	}
	return m_pAtlasBitmap->DrawBitmap(pDirect, INT_MAX, dstPoint);
}

bool CD2DFileIconDrawer::DrawFileIconBitmap(
	const CDirect2DWrite* pDirect,
	const CPointF& dstPoint,
	const CShellFile* pFile,
	std::function<void()>&& callback)
{
	if (!m_pAtlasBitmap->Exist(INT_MAX)) {
		m_pAtlasBitmap->AddOrAssign(pDirect, INT_MAX, GetBitmapSourceFromIcon(GetDefaultIcon()));
	}

	auto funadd = [pDirect, pFile, callback, this]()->void
	{
		CComPtr<IWICBitmapSource> pBitmap = GetBitmapSourceFromBitmap(pFile->GetIconBitmap());
		m_pAtlasBitmap->AddOrAssign(pDirect, pFile->GetIconFullIndex(), pBitmap);
		callback();
	};
	
	bool ret = false;
	if (!m_pAtlasBitmap->Exist(pFile->GetIconFullIndex())) {
		m_pAtlasBitmap->AddOrAssign(pDirect, pFile->GetIconFullIndex(), CComPtr<IWICBitmapSource>(nullptr));
		m_futureGroup.emplace_back(CThreadPool::GetInstance()->enqueue(
			FILE_LINE_FUNC,
			0,
			funadd));
		ret =  false;
	} else {
		ret =  m_pAtlasBitmap->DrawBitmap(pDirect, pFile->GetIconFullIndex(), dstPoint);
	}

	if (!ret) {
		m_pAtlasBitmap->DrawBitmap(pDirect, INT_MAX, dstPoint);
	}
	return ret;
}


void CD2DFileIconDrawer::Clear()
{
	m_pAtlasBitmap->Clear();
	m_futureGroup.clean_up();
}