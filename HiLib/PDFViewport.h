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
	CPointF CtrlToDoc(const CPointF& ptInCtrl);
	std::tuple<int, CPointF> DocToPage(const CPointF& ptInDoc);
	CPointF PageToPdfiumPage(const int& page, const CPointF& ptInPage);

	std::tuple<int, CPointF> WndToPdfiumPage(const CPointF& ptInWnd);

	CPointF PdfiumPageToPage(const int& page, const CPointF& ptInPdfiumPage);
	CPointF PageToDoc(const int& page, const CPointF& ptInPage);
	CPointF DocToCtrl(const CPointF& ptInDoc);
	CPointF CtrlToWnd(const CPointF& ptInCtrl);
	
	CPointF PageToWnd(const int& page, const CPointF ptInPage);
	CPointF PdfiumPageToWnd(const int& page, const CPointF ptInPdfiumPage);


	CRectF WndToCtrl(const CRectF& ptInWnd);
	CRectF CtrlToDoc(const CRectF& ptInWnd);
	//CRectF DocToPage(const CRectF& ptInDoc);
	//CRectF PageToPdfiumPage(const CRectF& ptInPage);

	//std::tuple<int, CRectF> WndToPdfiumPage(const CRectF& ptInWnd);

	CRectF PdfiumPageToPage(const int& page, const CRectF& rcInPdfiumPage);
	CRectF PageToDoc(const int& page, const CRectF& rcInPage);
	CRectF DocToCtrl(const CRectF& rcInDoc);
	CRectF CtrlToWnd(const CRectF& rcInCtrl);

	CRectF PdfiumPageToWnd(const int& page, const CRectF& rcInPdfiumPage);

};
