#pragma once
#include "D2DBitmapDrawer.h"
#include "PDFPage.h"

class CThreadPool;

struct PdfBmpKey
{
	CPDFPage* PagePtr;
	int HashCode;
	FLOAT Scale;
	int Rotate;
	CRectF Rect;

	//PdfBmpKey(CPDFPage* pPage, const FLOAT& scale, const int& rotate, const CRectF& rect)
	//:PagePtr(pPage), Scale(scale), Rotate(rotate), Rect(rect){}

	std::size_t GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(PagePtr)>()(PagePtr));
		boost::hash_combine(seed, std::hash<decltype(HashCode)>()(HashCode));
		boost::hash_combine(seed, std::hash<decltype(Scale)>()(Scale));
		boost::hash_combine(seed, std::hash<decltype(Rotate)>()(Rotate));
		boost::hash_combine(seed, std::hash<decltype(Rect.left)>()(Rect.left));
		boost::hash_combine(seed, std::hash<decltype(Rect.top)>()(Rect.top));
		boost::hash_combine(seed, std::hash<decltype(Rect.right)>()(Rect.right));
		boost::hash_combine(seed, std::hash<decltype(Rect.bottom)>()(Rect.bottom));
		return seed;
	}

	bool operator==(const PdfBmpKey& key) const
	{
		return 
			PagePtr == key.PagePtr &&
			HashCode == key.HashCode &&
			Scale == key.Scale &&
			Rotate == key.Rotate &&
			Rect == key.Rect;
	}

	CFPDFBitmap GetDefaultBitmap()const
	{
		return CFPDFBitmap();
	}

	template<typename _Cancel>
	CFPDFBitmap GetClipBitmap(_Cancel&& cancel)const
	{
		return PagePtr->GetClipFPDFBitmap(Scale, Rotate, Rect, cancel);
	}

	template<typename _Cancel>
	CFPDFBitmap GetBitmap(_Cancel&& cancel)const
	{
		return PagePtr->GetFPDFBitmap(Scale, Rotate, cancel);
	}
};

namespace std
{
	template <>
	struct hash<PdfBmpKey>
	{
		std::size_t operator() (PdfBmpKey const & key) const
		{
			return key.GetHashCode();
		}
	};
}

class CD2DPDFBitmapDrawer : public CD2DBitmapDrawer<PdfBmpKey>
{
	using CD2DBitmapDrawer::CD2DBitmapDrawer;

	//void AddClipBitmap(
	//	const CDirect2DWrite* pDirect,
	//	const PdfBmpKey& key,
	//	std::function<void()> callback) override;

	//void AddBitmap(
	//	const CDirect2DWrite* pDirect,
	//	const PdfBmpKey& key,
	//	std::function<void()> callback) override;

	//virtual void AddBlurBitmap(
	//	const CDirect2DWrite* pDirect,
	//	const PdfBmpKey& key,
	//	std::function<void()> callback) override;



};





//#pragma once
//#include "D2DAtlasBitmap.h"
//#include "D2DWTypes.h"
//#include "shared_lock_property.h"
//#include "getter_macro.h"
//
//class CPDFPage;
//class CFPDFPage;
//class CThreadPool;
//
//struct PdfBmpKey
//{
//	CPDFPage* PagePtr;
//	int HashCode;
//	FLOAT Scale;
//	int Rotate;
//	CRectF Rect;
//
//	//PdfBmpKey(CPDFPage* pPage, const FLOAT& scale, const int& rotate, const CRectF& rect)
//	//:PagePtr(pPage), Scale(scale), Rotate(rotate), Rect(rect){}
//
//	std::size_t GetHashCode() const
//	{
//		std::size_t seed = 0;
//		boost::hash_combine(seed, std::hash<decltype(PagePtr)>()(PagePtr));
//		boost::hash_combine(seed, std::hash<decltype(HashCode)>()(HashCode));
//		boost::hash_combine(seed, std::hash<decltype(Scale)>()(Scale));
//		boost::hash_combine(seed, std::hash<decltype(Rotate)>()(Rotate));
//		boost::hash_combine(seed, std::hash<decltype(Rect.left)>()(Rect.left));
//		boost::hash_combine(seed, std::hash<decltype(Rect.top)>()(Rect.top));
//		boost::hash_combine(seed, std::hash<decltype(Rect.right)>()(Rect.right));
//		boost::hash_combine(seed, std::hash<decltype(Rect.bottom)>()(Rect.bottom));
//		return seed;
//	}
//
//	bool operator==(const PdfBmpKey& key) const
//	{
//		return 
//			PagePtr == key.PagePtr &&
//			HashCode == key.HashCode &&
//			Scale == key.Scale &&
//			Rotate == key.Rotate &&
//			Rect == key.Rect;
//	}
//};
//
//namespace std
//{
//	template <>
//	struct hash<PdfBmpKey>
//	{
//		std::size_t operator() (PdfBmpKey const & key) const
//		{
//			return key.GetHashCode();
//		}
//	};
//}
//
//class CD2DPDFBitmapDrawer
//{
//	DECLARE_LAZY_GETTER(std::unique_ptr<CThreadPool>, ThreadPoolPtr);
//	DECLARE_LAZY_GETTER(CSizeU, PrimaryBitmapSize);
//
//private:
//	std::unique_ptr<CD2DAtlasBitmap<PdfBmpKey>> m_pAtlasFullOrClipBitmap;
//	std::unique_ptr<CD2DAtlasBitmap<PdfBmpKey>> m_pAtlasBlurBitmap;
//	shared_lock_property<PdfBmpKey> m_curKey;
//	shared_lock_property<PdfBmpKey> m_curClipKey;
//	future_group<void> m_futureGroup;
//
//public:
//	CD2DPDFBitmapDrawer();
//	~CD2DPDFBitmapDrawer();
//
//	bool DrawPDFPageClipBitmap(
//		const CDirect2DWrite* pDirect,
//		const PdfBmpKey& key,
//		const CPointF& dstPoint,
//		std::function<void()>&& callback);
//	bool DrawPDFPageBitmap(
//		const CDirect2DWrite* pDirect,
//		const PdfBmpKey& key,
//		const CPointF& dstRect,
//		std::function<void()>&& callback);
//	bool DrawPDFPageBlurBitmap(
//		const CDirect2DWrite* pDirect,
//		const PdfBmpKey& key,
//		const CRectF& dstRect,
//		std::function<void()>&& callback);
//
//	//int GetAtlasFullOrClipBitmapCount()const;
//	//int GetAtlasBlurBitmapCount()const;
//
//	const std::unique_ptr<CD2DAtlasBitmap<PdfBmpKey>>& GetAtlasFullOrClipBitmap() const { return m_pAtlasFullOrClipBitmap;}
//	const std::unique_ptr<CD2DAtlasBitmap<PdfBmpKey>>& GetAtlasBlurBitmap() const { return m_pAtlasBlurBitmap;}
//
//	bool ExistInFullOrClipBitmap(const PdfBmpKey&) const;
//	std::vector<PdfBmpKey> FindFulKeys(std::function<bool(const PdfBmpKey&)>&& pred);
//
//
//
//	void Clear();
//	//void CleanFutures();
//	void WaitAll();
//};