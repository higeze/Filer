#include "PDFBitmapDrawer.h"
#include "PDFPage.h"
#include "ThreadPool.h"
#include "D2DWTypes.h"
#include "FPDFBitmap.h"

//void CD2DPDFBitmapDrawer::AddClipBitmap(
//	const CDirect2DWrite* pDirect,
//	const PdfBmpKey& key,
//	std::function<void()> callback)
//{
//	do {
//		FALSE_BREAK(key == m_curClipKey.get());
//
//		CFPDFBitmap fpdfBmp = key.PagePtr->GetClipFPDFBitmap(key.Scale, key.Rotate, key.Rect, [this, key]()->bool {return key != m_curKey.get(); });
//		FALSE_BREAK(fpdfBmp);
//		m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, fpdfBmp);
//		callback();
//		return;
//
//	} while (1);
//
//	m_pAtlasPrimaryBitmap->Erase(key);
//}
//
//void CD2DPDFBitmapDrawer::AddBitmap(
//	const CDirect2DWrite* pDirect,
//	const PdfBmpKey& key,
//	std::function<void()> callback)
//{
//	do {
//		FALSE_BREAK(key == m_curKey.get());
//
//		CFPDFBitmap fpdfBmp = key.PagePtr->GetFPDFBitmap(key.Scale, key.Rotate, [this, key]()->bool {return key != m_curKey.get(); });
//		FALSE_BREAK(fpdfBmp);
//		m_pAtlasPrimaryBitmap->AddOrAssign(pDirect, key, fpdfBmp);
//
//		callback();
//		return;
//	} while (1);
//
//	m_pAtlasPrimaryBitmap->Erase(key);
//}
//
//void CD2DPDFBitmapDrawer::AddBlurBitmap(
//	const CDirect2DWrite* pDirect,
//	const PdfBmpKey& key,
//	std::function<void()> callback)
//{
//	do {
//		FALSE_BREAK(key == m_curBlurKey.get());
//
//		CFPDFBitmap fpdfBmp = key.PagePtr->GetFPDFBitmap(key.Scale, key.Rotate);
//		FALSE_BREAK(fpdfBmp);
//		m_pAtlasSecondaryBitmap->AddOrAssign(pDirect, key, fpdfBmp);
//
//		callback();
//		return;
//	} while (1);
//
//	m_pAtlasSecondaryBitmap->Erase(key);
//}

//#include "PDFPage.h"
//#include "ThreadPool.h"
//#include "D2DWTypes.h"
//#include "FPDFBitmap.h"
//
//
///***********************/
///* CD2DPDFBitmapDrawer */
///***********************/
//CD2DPDFBitmapDrawer::CD2DPDFBitmapDrawer()
//	:m_pAtlasFullOrClipBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
//		CSizeU(4096, 8196),
//		D2D1::BitmapProperties1(
//					D2D1_BITMAP_OPTIONS_NONE,
//					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))),
//	m_pAtlasBlurBitmap(std::make_unique<CD2DAtlasBitmap<PdfBmpKey>>(
//			CSizeU(2048, 4096),
//			D2D1::BitmapProperties1(
//						D2D1_BITMAP_OPTIONS_NONE,
//						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)))){}
//
//CD2DPDFBitmapDrawer::~CD2DPDFBitmapDrawer() = default;
//
//const std::unique_ptr<CThreadPool>& CD2DPDFBitmapDrawer::GetThreadPoolPtr() const
//{
//	if (!m_optThreadPoolPtr.has_value()) {
//		m_optThreadPoolPtr.emplace(std::make_unique<CThreadPool>(4));
//	}
//	return m_optThreadPoolPtr.value();
//}
//
//const CSizeU& CD2DPDFBitmapDrawer::GetPrimaryBitmapSize() const
//{
//	if (!m_optPrimaryBitmapSize.has_value()) {
//		m_optPrimaryBitmapSize.emplace(m_pAtlasFullOrClipBitmap->GetSize());
//	}
//	return m_optPrimaryBitmapSize.value();
//}
//
////bool CD2DPDFBitmapDrawer::DrawPDFPageBitmap(
////	const CDirect2DWrite* pDirect,
////	const PdfBmpKey& key,
////	const CRectF& dstRect,
////	std::function<void()>&& callback)
////{
////
////}
//
////bool CD2DPDFBitmapDrawer::DrawPDFPage(
////	const CDirect2DWrite* pDirect,
////	const PdfBmpKey& key,
////	const CRectF& dstRect,
////	std::function<void()>&& callback)
////{
////	CRectF scaledRect = key.Rect * key.Scale;
////	bool drawFullPage = (scaledRect.Width() * scaledRect.Height()) < (m_pAtlasClipBitmap->GetSize().width * m_pAtlasClipBitmap->GetSize().height / 8);
////
////	if (drawFullPage) {
////		return DrawPDFPageBitmap(pDirect, key, dstRect.LeftTop(), std::forward<std::function<void()>>(callback));
////	} else {
////		if (m_pAtlasClipBitmap->Exist(key)) {
////			return DrawPDFPageClipBitmap(pDirect, key, dstRect.LeftTop(), std::forward<std::function<void()>>(callback));
////		} else {
////			PdfBmpKey smallKey(key);
////			smallKey.Scale = 0.2f;
////			DrawPDFPageBitmap(pDirect, smallKey, dstRect, std::forward<std::function<void()>>(callback));
////			return DrawPDFPageClipBitmap(pDirect, key, dstRect.LeftTop(), std::forward<std::function<void()>>(callback));
////		}
////	}
////}
//
//bool CD2DPDFBitmapDrawer::DrawPDFPageClipBitmap(
//		const CDirect2DWrite* pDirect,
//		const PdfBmpKey& key,
//		const CPointF& dstPoint,
//		std::function<void()>&& callback)
//{
//	auto funadd = [pDirect, key, callback, this]()->void
//	{
//		do {
//			FALSE_BREAK(key == m_curClipKey.get());
//
//			CFPDFBitmap fpdfBmp = key.PagePtr->GetClipFPDFBitmap(key.Scale, key.Rotate, key.Rect, [this, key]()->bool {return key != m_curKey.get(); });
//			FALSE_BREAK(fpdfBmp);
//			m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, fpdfBmp);
//			callback();
//			return;
//
//		} while (1);
//
//		m_pAtlasFullOrClipBitmap->Erase(key);
//		return;
//	};
//	
//	if (!m_pAtlasFullOrClipBitmap->Exist(key)) {
//		m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, CFPDFBitmap());
//		m_curClipKey.set(key);
//		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0, funadd));
//		return false;
//	} else {
//		return m_pAtlasFullOrClipBitmap->DrawBitmap(pDirect, key, dstPoint);
//	}
//}
//
//bool CD2DPDFBitmapDrawer::DrawPDFPageBitmap(
//		const CDirect2DWrite* pDirect,
//		const PdfBmpKey& key,
//		const CPointF& dstPoint,
//		std::function<void()>&& callback)
//{
//	auto funadd = [pDirect, key, callback, this]()->void
//	{
//		do {
//			FALSE_BREAK(key == m_curKey.get());
//
//			CFPDFBitmap fpdfBmp = key.PagePtr->GetFPDFBitmap(key.Scale, key.Rotate, [this, key]()->bool {return key != m_curKey.get(); });
//			FALSE_BREAK(fpdfBmp);
//			m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, fpdfBmp);
//
//			callback();
//			return;
//		} while (1);
//
//		m_pAtlasFullOrClipBitmap->Erase(key);
//		return;
//	};
//	
//	if (!m_pAtlasFullOrClipBitmap->Exist(key)) {
//		m_pAtlasFullOrClipBitmap->AddOrAssign(pDirect, key, CFPDFBitmap());
//		m_curKey.set(key);
//		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 0, funadd));
//		return false;
//	} else {
//		return m_pAtlasFullOrClipBitmap->DrawBitmap(pDirect, key, dstPoint);
//	}
//}
//
//bool CD2DPDFBitmapDrawer::DrawPDFPageBlurBitmap(
//		const CDirect2DWrite* pDirect,
//		const PdfBmpKey& key,
//		const CRectF& dstRect,
//		std::function<void()>&& callback)
//{
//	auto funadd = [pDirect, key, callback, this]()->void
//	{
//		do {
//			CFPDFBitmap fpdfBmp = key.PagePtr->GetFPDFBitmap(key.Scale, key.Rotate);
//			FALSE_BREAK(fpdfBmp);
//			m_pAtlasBlurBitmap->AddOrAssign(pDirect, key, fpdfBmp);
//
//			callback();
//			return;
//		} while (1);
//
//		m_pAtlasBlurBitmap->Erase(key);
//		return;
//	};
//	
//	if (!m_pAtlasBlurBitmap->Exist(key)) {
//		m_pAtlasBlurBitmap->AddOrAssign(pDirect, key, CFPDFBitmap());
//		m_futureGroup.emplace_back(GetThreadPoolPtr()->enqueue(FILE_LINE_FUNC, 10, funadd));
//		return false;
//	} else {
//		return m_pAtlasBlurBitmap->DrawBitmap(pDirect, key, dstRect);
//	}
//}
//
//bool CD2DPDFBitmapDrawer::ExistInFullOrClipBitmap(const PdfBmpKey& key) const
//{
//	return m_pAtlasFullOrClipBitmap->Exist(key);
//}
//
//std::vector<PdfBmpKey> CD2DPDFBitmapDrawer::FindFulKeys(std::function<bool(const PdfBmpKey&)>&& pred)
//{
//	std::vector<PdfBmpKey> keys(m_pAtlasFullOrClipBitmap->Keys());
//	std::vector<PdfBmpKey> findKeys;
//	std::copy_if(keys.cbegin(), keys.cend(), std::back_inserter(findKeys), pred);
//	return findKeys;
//}
//
//void CD2DPDFBitmapDrawer::Clear()
//{
//	m_pAtlasFullOrClipBitmap->Clear();
//	m_pAtlasBlurBitmap->Clear();
//	m_futureGroup.clean_up();
//}
//
////void CD2DPDFBitmapDrawer::CleanFutures()
////{
////	m_futureGroup.clean_up();
////}
//
//void CD2DPDFBitmapDrawer::WaitAll()
//{
//	m_futureGroup.wait_all();
//}