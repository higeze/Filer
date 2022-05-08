#pragma once
#include "Direct2DWrite.h"
class CPdfView;

class CPDFViewport
{
private:
	CPdfView* m_pView;
public:
	CPDFViewport(CPdfView* pView)
		:m_pView(pView){}

	CPointF WndToCtrl(const CPointF& ptInWnd);
	CPointF CtrlToDoc(const CPointF& ptInCtrl, const FLOAT& scale);
	std::tuple<int, CPointF> DocToPage(const CPointF& ptInDoc);
	CPointF PageToPdfiumPage(const int& page, const CPointF& ptInPage);

	std::tuple<int, CPointF> WndToPdfiumPage(const CPointF& ptInWnd, const FLOAT& scale);

	//CRectF WndToCtrl(const CRectF& ptInWnd);
	//CRectF CtrlToDoc(const CRectF& ptInWnd);
	//CRectF DocToPage(const CRectF& ptInDoc);
	//CRectF PageToPdfiumPage(const CRectF& ptInPage);

	CRectF PdfiumPageToPage(const int& page, const CRectF& rcInPdfiumPage);
	CRectF PageToDoc(const int& page, const CRectF& rcInPage);
	CRectF DocToCtrl(const CRectF& rcInDoc, const FLOAT& scale);
	CRectF CtrlToWnd(const CRectF& rcInCtrl);

	CRectF PdfiumPageToWnd(const int& page, const CRectF& rcInPdfiumPage, const FLOAT& scale);

};
