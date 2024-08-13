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

struct ThumbnailBmpKey
{
	UINT32 Size;
	std::wstring Name;

	std::size_t GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(Size)>()(Size));
		boost::hash_combine(seed, std::hash<decltype(Name)>()(Name));
		return seed;
	}

	bool operator==(const ThumbnailBmpKey& key) const
	{
		return Size == key.Size && Name == key.Name;
	}
};

namespace std
{
	template <>
	struct hash<ThumbnailBmpKey>
	{
		std::size_t operator() (ThumbnailBmpKey const & key) const
		{
			return key.GetHashCode();
		}
	};
}

class CD2DThumbnailDrawer
{
private:
	std::unique_ptr<CD2DAtlasBitmap<ThumbnailBmpKey>> m_pAtlasBitmap;
	future_group<void> m_futureGroup;

public:
	CD2DThumbnailDrawer();
	~CD2DThumbnailDrawer() = default;
	CComPtr<IWICBitmapSource> GetThumbnailBitmapByThumbnailProvider(
		const std::wstring& dispName, 
		const UINT32& thumbSize);

	CComPtr<IWICBitmapSource> GetThumbnailBitmapByShellImageFactory(
		const std::wstring& dispName, 
		const CSize& thumbSize);

	CComPtr<IWICBitmapSource> GetThumbnailBitmapByThumbnailCache(
		const std::wstring& dispName, 
		const UINT32& thumbSize);

	CComPtr<IWICBitmapSource> GetIconBitmap(
		const std::wstring& dispName);

	bool DrawThumbnailBitmap(
		const CDirect2DWrite* pDirect,
		const ThumbnailBmpKey& key,
		const CPointF& dstPoint);
	void Clear();
};
