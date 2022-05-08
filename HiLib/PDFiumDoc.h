#pragma once

#include "Direct2DWrite.h"
#include "MyUniqueHandle.h"
#include <queue>
#include <future>
#include <boost/sml.hpp>

#include "PDFiumSingleThread.h"

#include "getter_macro.h"

struct PdfViewProperty;
class CPDFViewport;
class CPDFPage;


struct BitmapRenderEvent
{
	BitmapRenderEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const CRectF& rect, const FLOAT& scale)
	:DirectPtr(pDirect), ViewportPtr(pViewport), RenderRectInWnd(rect), Scale(scale){}
	virtual ~BitmapRenderEvent() = default;

	CDirect2DWrite* DirectPtr;
	CPDFViewport* ViewportPtr;
	CRectF RenderRectInWnd;
	FLOAT Scale;
};

struct FindRenderEvent:public BitmapRenderEvent
{
	FindRenderEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const CRectF& rect, const FLOAT& scale, const std::wstring& find)
	:BitmapRenderEvent(pDirect, pViewport, rect, scale), Find(find){}
	virtual ~FindRenderEvent() = default;

	std::wstring Find;
};

struct FindRenderLineEvent:public BitmapRenderEvent
{
	FindRenderLineEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const CRectF& rect, const FLOAT& scale, const std::wstring& find)
	:BitmapRenderEvent(pDirect, pViewport, rect, scale), Find(find){}
	virtual ~FindRenderLineEvent() = default;

	std::wstring Find;
};




class CPDFDoc
{
public:
	static void Init();
	static void Term();

private:
	std::shared_ptr<PdfViewProperty> m_pProp;
	CDirect2DWrite* m_pDirect;
	std::wstring m_path;
	std::wstring m_password;
	std::function<void()> m_changed;

	UNQ_FPDF_DOCUMENT m_pDoc;
	int m_pageCount;
	CSizeF m_sourceSize;
	std::vector<CRectF> m_sourceRectsInDoc;
	std::vector<std::unique_ptr<CPDFPage>> m_pages;
	std::unique_ptr<CPDFiumSingleThread> m_pPDFium;

public:
	CPDFDoc(
		const std::shared_ptr<PdfViewProperty>& spProp,
		const std::wstring& path,
		const std::wstring& password, 
		CDirect2DWrite* pDirect,
		std::function<void()> changed);
	virtual ~CPDFDoc();

	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pPDFium; }
	CDirect2DWrite* GetDirectPtr() { return m_pDirect; }
	std::shared_ptr<PdfViewProperty> GetPropPtr() const { return m_pProp; }
	std::vector<CRectF>& GetPageRects() { return m_sourceRectsInDoc; }
	UNQ_FPDF_DOCUMENT& GetDocPtr() { return m_pDoc; }
	int GetPageCount() const { return m_pageCount; }
	CSizeF GetSourceSize() const { return m_sourceSize; }

	std::vector<std::unique_ptr<CPDFPage>>& GetPages() { return m_pages; }

	std::unique_ptr<CPDFPage>& GetPage(int index)
	{
		FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
		return m_pages[index];
	}

	void RenderHighliteLine(const FindRenderLineEvent& e);


};



struct BitmapLoadEvent
{
	FLOAT Scale;
};
struct BitmapReloadEvent
{
	FLOAT Scale;
};
struct BitmapLoadCompletedEvent {};
struct BitmapCancelCompletedEvent {};
struct BitmapErrorEvent {};

struct FindLoadEvent 
{
	std::wstring Find;
};
struct FindReloadEvent
{
	std::wstring Find;
};
struct FindLoadCompletedEvent {};
struct FindCancelCompletedEvent {};
struct FindErrorEvent {};

struct PdfBmpInfo
{
	CComPtr<ID2D1Bitmap> BitmapPtr;
	FLOAT Scale;
};

struct PdfTxtInfo
{
	std::vector<CRectF> Rects;
	std::vector<CRectF> MouseRects;
	std::wstring String;
};

struct PdfFndInfo
{
	std::vector<CRectF> FindRects;
	std::wstring Find;
};

class CPDFPage
{
private:
	/* Field */
	CPDFDoc* m_pDoc;
	int m_index;

	PdfBmpInfo m_bmp;
	std::mutex m_mtxBmp;

	PdfFndInfo m_fnd;
	std::mutex m_mtxFnd;

	PdfTxtInfo m_txt;
	std::mutex m_mtxTxt;

	std::shared_ptr<bool> m_spCancelBitmapThread;
	std::shared_ptr<bool> m_spCancelFindThread;
	std::future<void> m_futureBitmap;
	std::future<void> m_futureFind;

	struct MachineBase;
	struct BitmapMachine;
	struct FindMachine;

	std::unique_ptr<boost::sml::sm<BitmapMachine, boost::sml::process_queue<std::queue>>> m_pBitmapMachine;
	std::unique_ptr<boost::sml::sm<FindMachine, boost::sml::process_queue<std::queue>>> m_pFindMachine;

	std::function<void()> StateChanged;
	FLOAT m_requestingScale;
	FLOAT m_loadingScale;



	UNQ_FPDF_PAGE m_pPage;
	CSizeF m_sourceSize;

public:
	/* Constructor/Destructor */
	CPDFPage(CPDFDoc* pDoc, int index);
	virtual ~CPDFPage();
	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pDoc->GetPDFiumPtr(); }

	/* Closure */
	//std::function<std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>& ()> GetPagePtr;
	//std::function<CSizeF()> GetSourceSize;
	UNQ_FPDF_PAGE& GetPagePtr() { return m_pPage; }
	CSizeF GetSourceSize() { return m_sourceSize; }
	/* Member function */
	void Render(const BitmapRenderEvent& e) { process_event(e); }
	void RenderHighlite(const FindRenderEvent& e) { process_event(e); }
	void RenderHighliteLine(const FindRenderLineEvent& e) { process_event(e); }

	int GetCursorCharIndexAtPos(const CPointF& ptInPdfiumPage);
	CRectF GetCursorRect(const int& index);

private:
	void LoadBitmap();
	void LoadText();
	void LoadFind(const std::wstring& find_string);

	const PdfBmpInfo& GetLockBitmap()
	{
		std::lock_guard<std::mutex> lock(m_mtxBmp);
		return m_bmp;
	}
	void SetLockBitmap(const PdfBmpInfo& bmp)
	{
		std::lock_guard<std::mutex> lock(m_mtxBmp);
		m_bmp = bmp;
	}
	const PdfTxtInfo& GetLockTxt()
	{
		std::lock_guard<std::mutex> lock(m_mtxTxt);
		return m_txt;
	}
	void SetLockTxt(const PdfTxtInfo& txt)
	{
		std::lock_guard<std::mutex> lock(m_mtxTxt);
		m_txt = txt;
	}
	const PdfFndInfo& GetLockFind()
	{
		std::lock_guard<std::mutex> lock(m_mtxFnd);
		return m_fnd;
	}
	void SetLockFind(const PdfFndInfo& fnd)
	{
		std::lock_guard<std::mutex> lock(m_mtxFnd);
		m_fnd = fnd;
	}


	virtual void process_event(const BitmapRenderEvent& e);
	virtual void process_event(const BitmapLoadEvent& e);
	virtual void process_event(const BitmapReloadEvent& e);
	virtual void process_event(const BitmapLoadCompletedEvent& e);
	virtual void process_event(const BitmapCancelCompletedEvent& e);
	virtual void process_event(const BitmapErrorEvent& e);

	virtual void process_event(const FindRenderEvent& e);
	virtual void process_event(const FindRenderLineEvent& e);
	virtual void process_event(const FindLoadEvent& e);
	virtual void process_event(const FindReloadEvent& e);
	virtual void process_event(const FindLoadCompletedEvent& e);
	virtual void process_event(const FindCancelCompletedEvent& e);
	virtual void process_event(const FindErrorEvent& e);


	void Bitmap_None_Render(const BitmapRenderEvent& e);
	void Bitmap_Loading_OnEntry();
	void Bitmap_Loading_Render(const BitmapRenderEvent& e);
	void Bitmap_Available_Render(const BitmapRenderEvent& e);
	void Bitmap_WaitCancel_OnEntry();
	void Bitmap_WaitCancel_OnExit();
	void Bitmap_WaitCancel_Render(const BitmapRenderEvent& e);
	void Bitmap_Error_Render(const BitmapRenderEvent& e);

	void Find_None_Render(const FindRenderEvent& e);
	void Find_None_RenderLine(const FindRenderLineEvent& e);
	void Find_Loading_OnEntry(const FindLoadEvent& e);
	void Find_Loading_OnReEntry(const FindReloadEvent& e);
	void Find_Loading_Render(const FindRenderEvent& e);
	void Find_Loading_RenderLine(const FindRenderLineEvent& e);
	void Find_Available_Render(const FindRenderEvent& e);
	void Find_Available_RenderLine(const FindRenderLineEvent& e);
	void Find_WaitCancel_OnEntry();
	void Find_WaitCancel_OnExit();
	void Find_WaitCancel_Render(const FindRenderEvent& e);
	void Find_WaitCancel_RenderLine(const FindRenderLineEvent& e);
	void Find_Error_Render(const FindRenderEvent& e);
	void Find_Error_RenderLine(const FindRenderLineEvent& e);
};

