#pragma once
#include "D2DAtlasBitmap.h"
#include "MyIcon.h"
#include "MyString.h"
#include <thumbcache.h>

namespace std
{
	template <>
	struct hash<CComPtr<IShellItem2>>
	{
		std::size_t operator() (const CComPtr<IShellItem2>& key) const
		{
			wchar_t* str;
			key->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &str);	
			std::size_t h =  hash<wstring>()(str);
			::CoTaskMemFree(str);
			return h;
		}
	};
}

class CD2DThumbnailDrawer
{
private:
	std::unique_ptr<CD2DAtlasBitmap<std::wstring>> m_pAtlasBitmap;
	std::vector<std::future<void>> m_futures;

public:
	CD2DThumbnailDrawer();
	~CD2DThumbnailDrawer() = default;
	CComPtr<ID2D1Bitmap> GetThumbnailBitmapByThumbnailProvider(
		const CDirect2DWrite* pDirect,
		const std::wstring& dispName, 
		const UINT32& thumbSize);

	CComPtr<ID2D1Bitmap1> GetThumbnailBitmapByShellImageFactory(
		const CDirect2DWrite* pDirect,
		const std::wstring& dispName, 
		const CSize& thumbSize);

	CComPtr<ID2D1Bitmap1> GetThumbnailBitmapByThumbnailCache(
		const CDirect2DWrite* pDirect,
		const std::wstring& dispName, 
		const UINT32& thumbSize);

	CComPtr<ID2D1Bitmap> GetIconBitmap(
		const CDirect2DWrite* pDirect,
		const std::wstring& dispName);

	void DrawThumbnailBitmap(
		const CDirect2DWrite* pDirect,
		const std::wstring& dispName,
		const CRectF& dstRect,
		const UINT& thumbSize);
	void Clear();
};
