#pragma once
#include <queue>
#include <future>
#include <vector>
#include <tuple>
#include <regex>
#include "Direct2DWrite.h"
#include "getter_macro.h"
#include "shared_lock_property.h"
#include "MyUniqueHandle.h"
#include "reactive_property.h"
#include "FPDFPage.h"
#include "FPDFTextPage.h"

class CFPDFBitmap;
class CPDFDoc;

using const_paragraph_iterator = std::tuple<std::wstring::const_iterator, std::wstring::const_iterator, std::wstring::const_iterator>;

////	std::wstring text = L"AAArnBBBrCCCnDDDrnEEErnrnGGGrn";
std::vector<const_paragraph_iterator> text_to_paragraph_iterators(const std::wstring& text);

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

class CPDFPage
{
/*******************/
/* member variable */
/*******************/
private:
	/* Field */
	const CPDFDoc* m_pDoc;
	int m_index;
/*********************/
/* reactive property */
/*********************/
public:
	std::shared_ptr<int> Dummy;
	mutable reactive_property_ptr<bool> IsDirty;

/***************/
/* lazy getter */
/***************/
	DECLARE_LAZY_GETTER(std::unique_ptr<CFPDFPage>, FPDFPagePtr);
	DECLARE_LAZY_GETTER(std::unique_ptr<CFPDFTextPage>, FPDFTextPagePtr);
	DECLARE_LAZY_GETTER(CSizeF, Size);
	DECLARE_LAZY_GETTER(std::wstring, Text);
	DECLARE_LAZY_GETTER(std::vector<const_paragraph_iterator>, ParagraphIterators);
	int GetTextSize() const;
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgCursorRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgMouseRects);	
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextCursorRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextMouseRects);	

protected:
	std::optional<PdfFndInfo> m_optFind;
public:
	const bool IsLoaded()const { return m_optFPDFPagePtr.has_value() && m_optFPDFTextPagePtr.has_value(); }
	void Unload() { m_optFPDFPagePtr.reset(), m_optFPDFTextPagePtr.reset(); }
	const std::vector<CRectF>& GetFindRects(const std::wstring& find);

public:
	int GetHashCode()const { return reinterpret_cast<int>(GetFPDFPagePtr().get()); }
	UHBITMAP GetClipBitmap(HDC hDC, const FLOAT& scale, const int& rotate, const CRectF& rect, std::function<bool()> cancel = []()->bool { return false; });

	CFPDFBitmap GetFPDFBitmap(const FLOAT& scale, const int& rotate, std::function<bool()> cancel = []()->bool { return false; });
	CFPDFBitmap GetClipFPDFBitmap(const FLOAT& scale, const int& rotate, const CRectF& rect, std::function<bool()> cancel = []()->bool { return false; });
	CFPDFBitmap GetThumbnailFPDFBitmap();

	UHBITMAP GetBitmap(HDC hDC, const FLOAT& scale, const int& rotate, std::function<bool()> cancel = []()->bool { return false; });
	UHBITMAP GetDDBitmap(HDC hDC, const FLOAT& scale, const int&, std::function<bool()> cancel = []()->bool { return false; });
	//CComPtr<IWICBitmap> GetWICBitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int&, std::function<bool()> cancel);
	CComPtr<ID2D1Bitmap1> GetD2D1Bitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int&, std::function<bool()> cancel = []()->bool { return false; });
	//void CopyImageToClipboard(HWND hWnd, HDC hDC, const FLOAT& scale, const int& rotate);

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
	CRectF RotateRect(const CRectF& rc, const int& rotate) const;
	void RotateRects(std::vector<CRectF>& rc, const int& rotate) const;

public:
	/* Constructor/Destructor */
	CPDFPage(const CPDFDoc* pDoc, int index);
	virtual ~CPDFPage();
	/* Reactive */
	reactive_property_ptr<int> Rotate;	
};

