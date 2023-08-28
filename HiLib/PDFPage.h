#pragma once
#include <queue>
#include <future>
#include "Direct2DWrite.h"
#include "getter_macro.h"
#include "shared_lock_property.h"
#include "MyUniqueHandle.h"
#include "reactive_property.h"

class CFPDFPage;
class CFPDFTextPage;
class CFPDFFormHandle;

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
	int GetTextSize() const;
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgCursorRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextCursorRects);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextOrgMouseRects);	
	DECLARE_LAZY_GETTER(std::vector<CRectF>, TextMouseRects);	

protected:
	std::optional<PdfFndInfo> m_optFind;
public:
	const std::vector<CRectF>& GetFindRects(const std::wstring& find);

public:
	int GetHashCode()const { return reinterpret_cast<int>(m_pPage.get()); }
	UHBITMAP GetClipBitmap(HDC hDC, const FLOAT& scale, const int& rotate, const CRectF& rect, std::function<bool()> cancel = []()->bool { return false; });
	UHBITMAP GetBitmap(HDC hDC, const FLOAT& scale, const int& rotate, std::function<bool()> cancel = []()->bool { return false; });

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

