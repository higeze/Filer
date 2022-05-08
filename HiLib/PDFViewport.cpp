#include "PDFViewport.h"
#include "PdfView.h"
#include "PDFiumDoc.h"

CPointF CPDFViewport::WndToCtrl(const CPointF& ptInWnd)
{
	CRectF rc = m_pView->GetRenderRectInWnd();
	return ptInWnd - rc.LeftTop();
}
CPointF CPDFViewport::CtrlToDoc(const CPointF& ptInCtrl, const FLOAT& scale)
{
	CPointF pt = CPointF(m_pView->GetHScrollPtr()->GetScrollPos(), m_pView->GetVScrollPtr()->GetScrollPos());
	return (ptInCtrl + pt) / scale;
}
std::tuple<int, CPointF> CPDFViewport::DocToPage(const CPointF& ptInDoc)
{
	auto& rects = m_pView->GetDocPtr()->GetPageRects();
	auto iter = std::find_if(rects.cbegin(), rects.cend(), [pt = ptInDoc](const CRectF& rc) { return rc.PtInRect(pt); });
	if (iter != rects.cend()) {
		return {std::distance(rects.cbegin(), iter), ptInDoc - iter->LeftTop()};
	} else {
		return {-1, CPointF()};
	}
}

CPointF CPDFViewport::PageToPdfiumPage(const int& page, const CPointF& ptInPage)
{
	auto sz = m_pView->GetDocPtr()->GetPage(page)->GetSourceSize();
	return CPointF(ptInPage.x, sz.height - ptInPage.y);
}

std::tuple<int, CPointF> CPDFViewport::WndToPdfiumPage(const CPointF& ptInWnd, const FLOAT& scale)
{
	auto ptInCtrl = WndToCtrl(ptInWnd);
	auto ptInDoc = CtrlToDoc(ptInCtrl, scale);
	auto [page, ptInPage] = DocToPage(ptInDoc);
	auto ptInPdfiumPage = PageToPdfiumPage(page, ptInPage);
	return { page, ptInPdfiumPage };
}
//
//CRectF CPDFViewport::WndToCtrl(const CRectF& ptInWnd);
//CRectF CPDFViewport::CtrlToDoc(const CRectF& ptInCtrl);
//CRectF CPDFViewport::DocToPage(const CRectF& ptInDoc);

CRectF CPDFViewport::PdfiumPageToPage(const int& page, const CRectF& rcInPdfiumPage)
{
	auto sz = m_pView->GetDocPtr()->GetPage(page)->GetSourceSize();
	return CRectF(
		rcInPdfiumPage.left,
		sz.height - rcInPdfiumPage.top,
		rcInPdfiumPage.right,
		sz.height - rcInPdfiumPage.bottom);
}
CRectF CPDFViewport::PageToDoc(const int& page, const CRectF& rcInPage)
{
	auto& rects = m_pView->GetDocPtr()->GetPageRects();
	return rcInPage + rects[page].LeftTop();
}
CRectF CPDFViewport::DocToCtrl(const CRectF& rcInDoc, const FLOAT& scale)
{
	CPointF pt = CPointF(m_pView->GetHScrollPtr()->GetScrollPos(), m_pView->GetVScrollPtr()->GetScrollPos());
	return (rcInDoc * scale) - pt;
}
CRectF CPDFViewport::CtrlToWnd(const CRectF& rcInCtrl)
{
	CRectF rc = m_pView->GetRenderRectInWnd();
	return rcInCtrl + rc.LeftTop();
}

CRectF CPDFViewport::PdfiumPageToWnd(const int& page, const CRectF& rcInPdfiumPage, const FLOAT& scale)
{
	auto rcInPage = PdfiumPageToPage(page, rcInPdfiumPage);
	auto rcInDoc = PageToDoc(page, rcInPage);
	auto rcInCtrl = DocToCtrl(rcInDoc, scale);
	return CtrlToWnd(rcInCtrl);
}