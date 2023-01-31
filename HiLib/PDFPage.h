#pragma once
#include "Direct2DWrite.h"
#include "PDFiumSingleThread.h"
#include <queue>
#include <future>
#include "ReactiveProperty.h"
#include "getter_macro.h"
#include "shared_lock_property.h"
#include <boost/sml.hpp>

class CPDFDoc;

enum class PdfBmpState
{
	New,
	Loading,
	Avail,
};
struct PdfBmpInfo
{
	CComPtr<ID2D1Bitmap1> BitmapPtr;
	FLOAT Scale;
	int Rotate;
	CRectF RectInPage;
};

struct PdfFndInfo
{
	std::wstring Find;
	std::vector<CRectF> FindRects;
};

struct SelectedTextInfo
{
	int Begin;
	int End;
	std::vector<CRectF> SelectedRects;
};

//struct GetBitmapEvent
//{
//	CDirect2DWrite* DirectPtr;
//	FLOAT Scale;
//	CRectF Rect;
//	int Rotate;
//	std::function<void()> Callback;
//};

class CPDFPage
{
private:
	/* Field */
	CPDFDoc* m_pDoc;
	int m_index;
	UNQ_FPDF_PAGE m_pPage;
	UNQ_FPDF_TEXTPAGE m_pTextPage;
	//struct BitmapMachine;
	//std::unique_ptr<boost::sml::sm<BitmapMachine, boost::sml::process_queue<std::queue>>> m_pBitmapMachine;
/**********/
/* Getter */
/**********/
public:
	int GetIndex() const { return m_index; }
/***************/
/* Lazy Getter */
/***************/
	DECLARE_LAZY_GETTER(CSizeF, Size);
	DECLARE_LAZY_GETTER(std::wstring, Text);
	int GetTextSize();
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgCursorRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextCursorRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgMouseRects);	
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextMouseRects);	


protected:
	std::optional<PdfFndInfo> m_optFind;
public:
	const std::vector<CRectF>& GetFindRects(const std::wstring& find);

//protected:
//	std::optional<PdfBmpInfo> m_optBitmapInfo;
public:
	CComPtr<ID2D1Bitmap1> GetClipBitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int& rotate, const CRectF& rect);
//protected:
//	std::optional<PdfBmpInfo> m_optSmallBitmap;
public:
	CComPtr<ID2D1Bitmap1> GetBitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int& rotate);

protected:
	std::optional<SelectedTextInfo> m_optSelectedText;
public:
	const std::vector<CRectF>& GetSelectedTextRects(const int& begin, const int& end);


/****************/
/* Other Getter */
/****************/

	CRectF GetCaretRect(const int index);
	int GetCursorCharIndexAtPos(const CPointF& ptInPdfiumPage);

private:
	CRectF RotateRect(const CRectF& rc, const int& rotate);
	void RotateRects(std::vector<CRectF>& rc, const int& rotate);

public:
	/* Constructor/Destructor */
	CPDFPage(CPDFDoc* pDoc, int index);
	virtual ~CPDFPage();
	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr();
	UNQ_FPDF_PAGE& GetPagePtr() { return m_pPage; }
	/* Reactive */
	ReactiveProperty<int> Rotate;	
};

