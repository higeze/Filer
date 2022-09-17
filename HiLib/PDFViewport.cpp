#include "PDFViewport.h"
#include "PdfView.h"
#include "PDFDoc.h"
#include "PDFPage.h"

CPointF CPDFViewport::WndToCtrl(const CPointF& ptInWnd)
{
	CRectF rc = m_pView->GetRenderRectInWnd();
	return ptInWnd - rc.LeftTop();
}
CPointF CPDFViewport::CtrlToDoc(const CPointF& ptInCtrl)
{
	CPointF pt = CPointF(m_pView->GetHScrollPtr()->GetScrollPos(), m_pView->GetVScrollPtr()->GetScrollPos());
	return (ptInCtrl + pt) / m_pView->GetScale();
}
std::tuple<int, CPointF> CPDFViewport::DocToPage(const CPointF& ptInDoc)
{
	auto& rects = m_pView->GetDocPtr()->GetSourceRectsInDoc();
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
CPointF CPDFViewport::PdfiumPageToPage(const int& page, const CPointF& ptInPdfiumPage)
{
	auto sz = m_pView->GetDocPtr()->GetPage(page)->GetSourceSize();
	return CPointF(
		ptInPdfiumPage.x,
		sz.height - ptInPdfiumPage.y
	);
}
CPointF CPDFViewport::PageToDoc(const int& page, const CPointF& ptInPage)
{
	auto& rects = m_pView->GetDocPtr()->GetSourceRectsInDoc();
	return ptInPage + rects[page].LeftTop();
}
CPointF CPDFViewport::DocToCtrl(const CPointF& ptInDoc)
{
	CPointF pt = CPointF(m_pView->GetHScrollPtr()->GetScrollPos(), m_pView->GetVScrollPtr()->GetScrollPos());
	return (ptInDoc * m_pView->GetScale()) - pt;
}
CPointF CPDFViewport::CtrlToWnd(const CPointF& ptInCtrl)
{
	CRectF rc = m_pView->GetRenderRectInWnd();
	return ptInCtrl + rc.LeftTop();
}

std::tuple<int, CPointF> CPDFViewport::WndToPdfiumPage(const CPointF& ptInWnd)
{
	try {
		auto ptInCtrl = WndToCtrl(ptInWnd);
		auto ptInDoc = CtrlToDoc(ptInCtrl);
		auto [page, ptInPage] = DocToPage(ptInDoc);
		auto ptInPdfiumPage = PageToPdfiumPage(page, ptInPage);
		return { page, ptInPdfiumPage };
	}
	catch(...){
		return { -1, CPointF() };
	}
}

CPointF CPDFViewport::PageToWnd(const int& page, const CPointF ptInPage)
{
	auto ptInDoc = PageToDoc(page, ptInPage);
	auto ptInCtrl = DocToCtrl(ptInDoc);
	return CtrlToWnd(ptInCtrl);
}

CPointF CPDFViewport::PdfiumPageToWnd(const int& page, const CPointF ptInPdfiumPage)
{
	auto ptInPage = PdfiumPageToPage(page, ptInPdfiumPage);
	auto ptInDoc = PageToDoc(page, ptInPage);
	auto ptInCtrl = DocToCtrl(ptInDoc);
	return CtrlToWnd(ptInCtrl);
}

CRectF CPDFViewport::WndToCtrl(const CRectF& rcInWnd)
{
	CRectF rc = m_pView->GetRenderRectInWnd();
	return rcInWnd - rc.LeftTop();
}
CRectF CPDFViewport::CtrlToDoc(const CRectF& rcInCtrl)
{
	CPointF pt = CPointF(m_pView->GetHScrollPtr()->GetScrollPos(), m_pView->GetVScrollPtr()->GetScrollPos());
	return (rcInCtrl + pt) / m_pView->GetScale();
}
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
	auto& rects = m_pView->GetDocPtr()->GetSourceRectsInDoc();
	return rcInPage + rects[page].LeftTop();
}
CRectF CPDFViewport::DocToCtrl(const CRectF& rcInDoc)
{
	CPointF pt = CPointF(m_pView->GetHScrollPtr()->GetScrollPos(), m_pView->GetVScrollPtr()->GetScrollPos());
	return (rcInDoc * m_pView->GetScale()) - pt;
}
CRectF CPDFViewport::CtrlToWnd(const CRectF& rcInCtrl)
{
	CRectF rc = m_pView->GetRenderRectInWnd();
	return rcInCtrl + rc.LeftTop();
}

CRectF CPDFViewport::PdfiumPageToWnd(const int& page, const CRectF& rcInPdfiumPage)
{
	auto rcInPage = PdfiumPageToPage(page, rcInPdfiumPage);
	auto rcInDoc = PageToDoc(page, rcInPage);
	auto rcInCtrl = DocToCtrl(rcInDoc);
	return CtrlToWnd(rcInCtrl);
}