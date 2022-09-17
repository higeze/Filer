#pragma once
#include "Direct2DWrite.h"
#include "PDFiumSingleThread.h"
#include "PDFEvent.h"

#include <queue>
#include <future>
#include <boost/sml.hpp>
#include "ReactiveProperty.h"

#include "getter_macro.h"

class CPDFDoc;

struct InitialLoadEvent
{
	InitialLoadEvent(CDirect2DWrite* pDirect, const FLOAT& scale)
		:DirectPtr(pDirect), Scale(scale){}
	CDirect2DWrite* DirectPtr;
	FLOAT Scale;
};

struct BitmapReloadEvent
{
	BitmapReloadEvent(CDirect2DWrite* pDirect, const FLOAT& scale)
		:DirectPtr(pDirect), Scale(scale){}
	CDirect2DWrite* DirectPtr;
	FLOAT Scale;
};

struct BitmapLoadCompletedEvent 
{
	BitmapLoadCompletedEvent(CDirect2DWrite* pDirect, const FLOAT& scale)
		:DirectPtr(pDirect), Scale(scale){}
	CDirect2DWrite* DirectPtr;
	FLOAT Scale;
};

struct BitmapCancelCompletedEvent 
{
	BitmapCancelCompletedEvent(CDirect2DWrite* pDirect, const FLOAT& scale)
		:DirectPtr(pDirect), Scale(scale){}
	CDirect2DWrite* DirectPtr;
	FLOAT Scale;
};

struct BitmapErrorEvent 
{
	FLOAT Scale;
};

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
	int Rotate;
};

struct PdfTxtInfo
{
	std::vector<CRectF> Rects;
	std::vector<CRectF> CRLFRects;
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
	//FLOAT m_requestingScale;
	FLOAT m_loadingScale;
	int m_loadingRotate;
	//int m_rotate;



	UNQ_FPDF_PAGE m_pPage;
	UNQ_FPDF_TEXTPAGE m_pTextPage;
	CSizeF m_sourceSize;

	LAZY_GETTER(CSizeF, SourceSize)

public:
	/* Constructor/Destructor */
	CPDFPage(CPDFDoc* pDoc, int index);
	virtual ~CPDFPage();
	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr();
	/* Reactive */
	ReactiveProperty<int> Rotate;	
	/* Closure */

	UNQ_FPDF_PAGE& GetPagePtr() { return m_pPage; }

	/* Member function */
	void RenderContent(const RenderPageContentEvent& e);
	void RenderFind(const RenderPageFindEvent& e);
	void RenderFindLine(const RenderPageFindLineEvent& e);
	void RenderSelectedText(const RenderPageSelectedTextEvent& e);
	void RenderCaret(const RenderPageCaretEvent& e);

	int GetCursorCharIndexAtPos(const CPointF& ptInPdfiumPage);
	CRectF GetCursorRect(const int& index);
	int GetTextSize();
	std::wstring GetText();

private:
	void LoadBitmap(CDirect2DWrite* pDirect, const FLOAT& scale/*, const int& rotate*/);
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


	virtual void process_event(const RenderPageContentEvent& e);
	virtual void process_event(const RenderPageFindEvent& e);
	virtual void process_event(const RenderPageFindLineEvent& e);
	virtual void process_event(const RenderPageCaretEvent& e);

	virtual void process_event(const InitialLoadEvent& e);
	virtual void process_event(const BitmapReloadEvent& e);
	virtual void process_event(const BitmapLoadCompletedEvent& e);
	virtual void process_event(const BitmapCancelCompletedEvent& e);
	virtual void process_event(const BitmapErrorEvent& e);

	virtual void process_event(const FindLoadEvent& e);
	virtual void process_event(const FindReloadEvent& e);
	virtual void process_event(const FindLoadCompletedEvent& e);
	virtual void process_event(const FindCancelCompletedEvent& e);
	virtual void process_event(const FindErrorEvent& e);

	virtual void process_event(const RenderPageSelectedTextEvent& e);


	void Bitmap_None_Render(const RenderPageContentEvent& e);
	void Bitmap_InitialLoading_OnEntry(const InitialLoadEvent& e);
	void Bitmap_Loading_OnEntry(CDirect2DWrite* pDirect, const FLOAT& scale);
	void Bitmap_Loading_Render(const RenderPageContentEvent& e);
	void Bitmap_Available_Render(const RenderPageContentEvent& e);
	void Bitmap_Available_RenderSelectedText(const RenderPageSelectedTextEvent& e);
	void Bitmap_Available_RenderCaret(const RenderPageCaretEvent& e);
	void Bitmap_WaitCancel_OnEntry();
	void Bitmap_WaitCancel_OnExit();
	void Bitmap_WaitCancel_Render(const RenderPageContentEvent& e);
	void Bitmap_Error_Render(const RenderPageContentEvent& e);

	void Find_None_Render(const RenderPageFindEvent& e);
	void Find_None_RenderLine(const RenderPageFindLineEvent& e);
	void Find_Loading_OnEntry(const FindLoadEvent& e);
	void Find_Loading_OnReEntry(const FindReloadEvent& e);
	void Find_Loading_Render(const RenderPageFindEvent& e) {}
	void Find_Loading_RenderLine(const RenderPageFindLineEvent& e) {}
	void Find_Available_Render(const RenderPageFindEvent& e);
	void Find_Available_RenderLine(const RenderPageFindLineEvent& e);
	void Find_WaitCancel_OnEntry();
	void Find_WaitCancel_OnExit();
	void Find_WaitCancel_Render(const RenderPageFindEvent& e) {}
	void Find_WaitCancel_RenderLine(const RenderPageFindLineEvent& e) {}
	void Find_Error_Render(const RenderPageFindEvent& e) {}
	void Find_Error_RenderLine(const RenderPageFindLineEvent& e) {}
};

