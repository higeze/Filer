#pragma once
#include "D2DAtlasBitmap.h"
#include "D2DImage.h"
#include "D2DWTypes.h"
#include "shared_lock_property.h"
#include "getter_macro.h"

class CThreadPool;
class CD2DImage;

struct ImgBmpKey
{
	const CD2DImage* ImagePtr;
	FLOAT Scale;
	int Rotate;
	CRectU Rect;

	//PdfBmpKey(CPDFPage* pPage, const FLOAT& scale, const int& rotate, const CRectF& rect)
	//:PagePtr(pPage), Scale(scale), Rotate(rotate), Rect(rect){}

	std::size_t GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<std::wstring>()(ImagePtr->GetPath()));
		boost::hash_combine(seed, std::hash<decltype(Scale)>()(Scale));
		boost::hash_combine(seed, std::hash<decltype(Rotate)>()(Rotate));
		boost::hash_combine(seed, std::hash<decltype(Rect.left)>()(Rect.left));
		boost::hash_combine(seed, std::hash<decltype(Rect.top)>()(Rect.top));
		boost::hash_combine(seed, std::hash<decltype(Rect.right)>()(Rect.right));
		boost::hash_combine(seed, std::hash<decltype(Rect.bottom)>()(Rect.bottom));
		return seed;
	}

	bool operator==(const ImgBmpKey& key) const
	{
		return 
			ImagePtr->GetPath() == key.ImagePtr->GetPath() &&
			Scale == key.Scale &&
			Rotate == key.Rotate &&
			Rect == key.Rect;
	}
};

namespace std
{
	template <>
	struct hash<ImgBmpKey>
	{
		std::size_t operator() (ImgBmpKey const & key) const
		{
			return key.GetHashCode();
		}
	};
}

class CImageDrawer
{
	DECLARE_LAZY_GETTER(CSizeU, PrimaryBitmapSize);
	DECLARE_LAZY_GETTER(std::unique_ptr<CThreadPool>, ThreadPoolPtr);

private:
	std::unique_ptr<CD2DAtlasBitmap<ImgBmpKey>> m_pAtlasClipBitmap;
	std::unique_ptr<CD2DAtlasBitmap<ImgBmpKey>> m_pAtlasSmallBitmap;
	shared_lock_property<ImgBmpKey> m_curClipKey;
	future_group<void> m_futureGroup;
public:
	CImageDrawer();
	~CImageDrawer();

	const ImgBmpKey& GetCurClipKey() { return m_curClipKey.get(); }

	bool DrawClipBitmap(
		const CDirect2DWrite* pDirect,
		const ImgBmpKey& key,
		const CPointF& dstPoint,
		std::function<void()>&& callback);
	bool DrawClipBitmap(
		const CDirect2DWrite* pDirect,
		const ImgBmpKey& key,
		const CRectF& dstRect,
		std::function<void()>&& callback);
	bool DrawBitmap(
		const CDirect2DWrite* pDirect,
		const ImgBmpKey& key,
		const CPointF& dstRect,
		std::function<void()>&& callback);
	bool DrawBitmap(
		const CDirect2DWrite* pDirect,
		const ImgBmpKey& key,
		const CRectF& dstRect,
		std::function<void()>&& callback);

	bool ExistInPrimary(const ImgBmpKey&) const;
	std::vector<ImgBmpKey> FindClipKeys(std::function<bool(const ImgBmpKey&)>&& pred);

	void Clear();
	//void CleanFutures();
	void WaitAll();
};