#include "D2DFileIconDrawer.h"
#include "ThreadPool.h"
#include "ShellFile.h"

/************************/
/* CShellFileIconDrawer */
/************************/
CD2DFileIconDrawer::CD2DFileIconDrawer()
	:m_pAtlasBitmap(std::make_unique<CD2DAtlasBitmap<std::wstring>>(
		CSizeU(256, 256),
		D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))),
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

CComPtr<ID2D1Bitmap1> CD2DFileIconDrawer::GetBitmapFromIcon(const CDirect2DWrite* pDirect, const CIcon& icon) const
{
	CComPtr<IWICBitmap> pWICBitmap;
	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapFromHICON(icon, &pWICBitmap));
	CComPtr<IWICFormatConverter> pWICFormatConverter;
	FAILED_THROW(pDirect->GetWICImagingFactory()->CreateFormatConverter(&pWICFormatConverter));
	FAILED_THROW(pWICFormatConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut));
	CComPtr<ID2D1Bitmap1> pBitmap;
	FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICFormatConverter, nullptr, &pBitmap));
	return pBitmap;
}

bool CD2DFileIconDrawer::DrawFileIconBitmap(
	const CDirect2DWrite* pDirect,
	const CPointF& dstPoint,
	const CIDL& absoluteIDL,
	const std::wstring& path,
	const std::wstring& ext,
	const DWORD& data,
	std::function<void()>&& callback)
{
	std::wstring key = L"DEFAULT";
	if (!m_pAtlasBitmap->Exist(key)) {
		m_pAtlasBitmap->AddOrAssign(pDirect, key, GetBitmapFromIcon(pDirect, GetDefaultIcon()));
	}

	if (!absoluteIDL || path.empty()) {
		key = L"DEFAULT";
	} else if (!ext.empty() && m_excludeExtSet.find(ext) == m_excludeExtSet.end() && data != 0) {
		key = ext;

	} else {
		key = path;
	}
	auto funadd = [pDirect, key, absoluteIDL, callback, this]()->void
	{
		CComPtr<ID2D1Bitmap1> pBitmap = GetBitmapFromIcon(pDirect, GetIcon(absoluteIDL));
		m_pAtlasBitmap->AddOrAssign(pDirect, key, pBitmap);
		callback();
	};
	
	bool ret = false;
	if (!m_pAtlasBitmap->Exist(key)) {
		m_pAtlasBitmap->AddOrAssign(pDirect, key, CComPtr<ID2D1Bitmap1>(nullptr));
		m_futureGroup.emplace_back(CThreadPool::GetInstance()->enqueue(funadd, 0));
		ret =  false;
	} else {
		ret =  m_pAtlasBitmap->DrawBitmap(pDirect, key, dstPoint);
	}

	if (!ret) {
		m_pAtlasBitmap->DrawBitmap(pDirect, L"DEFAULT", dstPoint);
	}
	return ret;


	//auto funbmp = [absoluteIDL, this]()->CComPtr<ID2D1Bitmap>
	//{
	//	return GetBitmapFromIcon(GetIcon(absoluteIDL));
	//};

	//std::wstring key = L"DEFAULT";
	//if (!m_pAtlasBitmap->Exist(key)) {
	//	m_pAtlasBitmap->AddOrAssign(key, GetBitmapFromIcon(GetDefaultIcon()));
	//}


	//if (!absoluteIDL || path.empty()) {
	//	//Default Icon
	//} else if (!ext.empty() && m_excludeExtSet.find(ext) == m_excludeExtSet.end() && data != 0) {
	//	key = ext;

	//} else {
	//	key = path;
	//}

	//if (!m_pAtlasBitmap->Exist(key)) {
	//	//::OutputDebugString(std::format(L"{}\r\n", key).c_str());
	//	m_pAtlasBitmap->AddAsync(key, funbmp, std::forward<decltype(funupd)>(funupd), L"DEFAULT");
	//}
	//m_pAtlasBitmap->DrawBitmapByKey(key, dstRect);
}

bool CD2DFileIconDrawer::DrawFileIconBitmap(
	const CDirect2DWrite* pDirect,
	const CPointF& dstPoint,
	const CShellFile* pFile,
	std::function<void()>&& callback)
{
	if (!m_pAtlasBitmap->Exist(L"DEFAULT")) {
		m_pAtlasBitmap->AddOrAssign(pDirect, L"DEFAULT", GetBitmapFromIcon(pDirect, GetDefaultIcon()));
	}

	auto funadd = [pDirect, pFile, callback, this]()->void
	{
		CComPtr<ID2D1Bitmap1> pBitmap = GetBitmapFromIcon(pDirect, pFile->GetIcon());
		m_pAtlasBitmap->AddOrAssign(pDirect, pFile->GetIconKey(), pBitmap);
		callback();
	};
	
	bool ret = false;
	if (!m_pAtlasBitmap->Exist(pFile->GetIconKey())) {
		m_pAtlasBitmap->AddOrAssign(pDirect, pFile->GetIconKey(), CComPtr<ID2D1Bitmap1>(nullptr));
		m_futureGroup.emplace_back(CThreadPool::GetInstance()->enqueue(funadd, 0));
		ret =  false;
	} else {
		ret =  m_pAtlasBitmap->DrawBitmap(pDirect, pFile->GetIconKey(), dstPoint);
	}

	if (!ret) {
		m_pAtlasBitmap->DrawBitmap(pDirect, L"DEFAULT", dstPoint);
	}
	return ret;
}


void CD2DFileIconDrawer::Clear()
{
	m_pAtlasBitmap->Clear();
	m_futureGroup.clean_up();
}