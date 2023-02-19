#include "D2DAtlasBitmap.h"
#include "MyCom.h"
#include "Debug.h"
#include <VersionHelpers.h>
#include <ntddkbd.h>
#include "ThreadPool.h"



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
/***********************/
/* CD2DAtlasBitmapBase */
/***********************/
CD2DAtlasBitmapBase::CD2DAtlasBitmapBase(CDirect2DWrite* pDirect, const CSizeU& size)
	:m_pDirect(pDirect), m_size(size){}

const CComPtr<ID2D1Bitmap>& CD2DAtlasBitmapBase::GetAtlasBitmapPtr() 
{ 
	if (!m_pAtlasBitmap) { LoadAtlasBitmapPtr(); }
	return m_pAtlasBitmap;
}

void CD2DAtlasBitmapBase::LoadAtlasBitmapPtr()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	m_pDirect->GetD2DDeviceContext()->CreateBitmap(
        m_size,
        D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
        &m_pAtlasBitmap
	);
}
void CD2DAtlasBitmapBase::ClearAtlasBitmapPtr() 
{
	std::lock_guard<std::mutex> lock(m_mtx);
	m_pAtlasBitmap = nullptr;
}

void CD2DAtlasBitmapBase::DrawAtlasBitmap(const CRectF& dstRect, const CRectF& srcRect)
{
	auto rect = CRectF(
	std::round(dstRect.left),
	std::round(dstRect.top),
	std::round(dstRect.right),
	std::round(dstRect.bottom));
	return m_pDirect->GetD2DDeviceContext()->DrawBitmap(
		GetAtlasBitmapPtr(),
		rect,
		1.f,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		srcRect);
}

void CD2DAtlasBitmapBase::DrawAtlasBitmap(const CRectF& dstRect, const CRectU& srcRect)
{
	return DrawAtlasBitmap(dstRect, CRectU2CRectF(srcRect));
}

/****************************/
/* CD2DAtlasFixedSizeBitmap */
/****************************/
CD2DAtlasFixedSizeBitmap::CD2DAtlasFixedSizeBitmap(CDirect2DWrite* pDirect, const CSizeU& atlasBitmapSize, const CSizeU& oneBitmapSize )
	:CD2DAtlasBitmapBase(pDirect, atlasBitmapSize), m_oneSize(oneBitmapSize){}
CRectU CD2DAtlasFixedSizeBitmap::GetPlacement(const int& index)
{
	auto left = (index % (m_size.width / m_oneSize.width)) * m_oneSize.width;
	auto top = (index / (m_size.width / m_oneSize.width)) * m_oneSize.height;
	return D2D1::RectU(left, top, left + m_oneSize.width, top + m_oneSize.height);
}

void CD2DAtlasFixedSizeBitmap::Add(const std::wstring& key, const CComPtr<ID2D1Bitmap>& pBitmap)
{
	CSizeU size = GetAtlasBitmapPtr()->GetPixelSize();

	std::lock_guard<std::mutex> lock(m_mtx);
	if (m_indexSize == (size.width / m_oneSize.width) * (size.height / m_oneSize.height)) {
		Clear();
	}
	CRectU placement = GetPlacement(m_indexSize);
	CPointU lefttop = placement.LeftTop();
	CRectU oneRect(m_oneSize);
	GetAtlasBitmapPtr()->CopyFromBitmap(&lefttop, pBitmap, &oneRect);
	m_map.insert_or_assign(key, placement);
	m_indexSize++;
}

void CD2DAtlasFixedSizeBitmap::AddAsync(const std::wstring& key, std::function<CComPtr<ID2D1Bitmap>()>&& funbmp, std::function<void()>&& funupd, const std::wstring& defKey )
{
	std::lock_guard<std::mutex> lock(m_mtx);
	m_map.emplace(key, m_map[defKey]);

	auto funadd = [key, funbmp, funupd, this]()
	{
		CComPtr<ID2D1Bitmap> pBitmap = funbmp();
		Add(key, pBitmap);
		if (funupd) {
			funupd();
		}
	};
	m_futures.emplace_back(CThreadPool::GetInstance()->enqueue(funadd, 0));
}
bool CD2DAtlasFixedSizeBitmap::Exist(const std::wstring& key)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_map.find(key) != m_map.end();
}
void CD2DAtlasFixedSizeBitmap::Clear()
{
	m_map.clear();
}
void CD2DAtlasFixedSizeBitmap::DrawBitmapByKey(const std::wstring& key, const CRectF& dstRect)
{
	auto iter = m_map.find(key);
	if (iter != m_map.end()) {
		DrawAtlasBitmap(dstRect, iter->second);
	}
}

