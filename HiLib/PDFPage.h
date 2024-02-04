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

class CFPDFPage;
class CFPDFBitmap;
class CFPDFTextPage;
class CFPDFFormHandle;

using const_paragraph_iterator = std::tuple<std::wstring::const_iterator, std::wstring::const_iterator, std::wstring::const_iterator>;

////	std::wstring text = L"AAArnBBBrCCCnDDDrnEEErnrnGGGrn";

std::vector<const_paragraph_iterator> text_to_paragraph_iterators(const std::wstring& text);
//{
//	using const_iterator = std::wstring::const_iterator;
//	std::vector<std::tuple<const_iterator, const_iterator, const_iterator>> paras;
//	const_iterator beg = text.cbegin();
//	const_iterator end = text.cend();
//	std::wregex re{L"rn|r|n"};
//	const_iterator para_prev_end = text.begin();
//	for (std::wsregex_iterator re_iter{beg, beg, re}, re_end; re_iter != re_end; ++re_iter) {
//		const_iterator para_beg = para_prev_end;
//		const_iterator para_crlfbeg = std::next(beg, re_iter->position());
//		const_iterator para_end = std::next(beg, re_iter->position() + re_iter->length());
//		paras.emplace_back(para_beg, para_crlfbeg, para_end);
//		para_prev_end = para_end;
//	}
//	if (para_prev_end != end) {
//		paras.emplace_back(para_prev_end, end, end);
//	}
//	return paras;
//}


//class CPDFText
//{
//public:
//private:
//	std::unique_ptr<CFPDFTextPage> m_pTextPage;
//public:
//	CPDFText(std::unique_ptr<CFPDFTextPage>&& pTextPage);
//	virtual ~CPDFText() = default;
//
//	DECLARE_LAZY_GETTER(std::wstring, Text);
//	DECLARE_LAZY_GETTER(std::vector<const_paragraph_iterator>, ParagraphIterators);
//	int GetTextSize() const;
//	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgRects);
//	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgCursorRects);
//	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextCursorRects);
//	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgMouseRects);	
//	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextMouseRects);	
//};
//
//class CPDFParagraph
//{
//	std::size_t 
//}


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
	std::unique_ptr<CFPDFPage> m_pPage;
	std::unique_ptr<CFPDFTextPage> m_pTextPage;
	std::shared_ptr<CFPDFFormHandle> m_pForm;
/**********/
/* Getter */
/**********/
public:
/************/
/* Reactive */
/************/
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<bool> IsDirty;

/***************/
/* Lazy Getter */
/***************/
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
	const std::vector<CRectF>& GetFindRects(const std::wstring& find);

public:
	int GetHashCode()const { return reinterpret_cast<int>(m_pPage.get()); }
	UHBITMAP GetClipBitmap(HDC hDC, const FLOAT& scale, const int& rotate, const CRectF& rect, std::function<bool()> cancel = []()->bool { return false; });

	CFPDFBitmap GetFPDFBitmap(const FLOAT& scale, const int& rotate, std::function<bool()> cancel = []()->bool { return false; });
	CFPDFBitmap GetClipFPDFBitmap(const FLOAT& scale, const int& rotate, const CRectF& rect, std::function<bool()> cancel = []()->bool { return false; });

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
	CPDFPage(std::unique_ptr<CFPDFPage>&& pPage, std::unique_ptr<CFPDFTextPage>&& pTextPage, const std::shared_ptr<CFPDFFormHandle>& pForm);
	virtual ~CPDFPage();
	/* Reactive */
	reactive_property_ptr<int> Rotate;	
};

