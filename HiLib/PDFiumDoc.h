#pragma once

#include "Direct2DWrite.h"
#include "MyUniqueHandle.h"
#include <queue>
#include <future>
#include <boost/sml.hpp>

#include "PDFiumSingleThread.h"

struct PdfViewProperty;
class CPDFiumPage;


class CPDFiumDoc
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
	std::vector<std::unique_ptr<CPDFiumPage>> m_pages;
	std::unique_ptr<CPDFiumSingleThread> m_pPDFium;

public:
	CPDFiumDoc(
		const std::shared_ptr<PdfViewProperty>& spProp,
		const std::wstring& path,
		const std::wstring& password, 
		CDirect2DWrite* pDirect,
		std::function<void()> changed);
	virtual ~CPDFiumDoc();

	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pPDFium; }
	CDirect2DWrite* GetDirectPtr() { return m_pDirect; }
	std::shared_ptr<PdfViewProperty> GetPropPtr() const { return m_pProp; }

	UNQ_FPDF_DOCUMENT& GetDocPtr() { return m_pDoc; }
	int GetPageCount() const { return m_pageCount; }
	CSizeF GetSourceSize() const { return m_sourceSize; }
	std::vector<std::unique_ptr<CPDFiumPage>>& GetPages() { return m_pages; }

	std::unique_ptr<CPDFiumPage>& GetPage(int index)
	{
		FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
		return m_pages[index];
	}
};

struct RenderEvent
{
	RenderEvent(CDirect2DWrite* pDirect, const CRectF& rect, const FLOAT& scale)
	:DirectPtr(pDirect), RenderRectInWnd(rect), Scale(scale){}
	virtual ~RenderEvent() = default;

	CDirect2DWrite* DirectPtr;
	CRectF RenderRectInWnd;
	FLOAT Scale;
};
struct LoadEvent {};
struct ReloadEvent {};
struct LoadCompletedEvent {};
struct CancelCompletedEvent {};
struct ErrorEvent {};

struct PdfBmpInfo
{
	CComPtr<ID2D1Bitmap> BitmapPtr;
	FLOAT Scale;
};

class CPDFiumPage
{
private:
	/* Field */
	CPDFiumDoc* m_pDoc;
	int m_index;
	PdfBmpInfo m_bmp;
	std::mutex m_mtxBmp;
	std::shared_ptr<bool> m_spCancelThread;
	std::future<void> m_future;

	struct Machine;

	std::unique_ptr<boost::sml::sm<Machine, boost::sml::process_queue<std::queue>>> m_pMachine;
	std::function<void()> StateChanged;
	FLOAT m_requestingScale;
	FLOAT m_loadingScale;



	UNQ_FPDF_PAGE m_pPage;
	CSizeF m_sourceSize;

	std::vector<CRectF> m_findRects;

public:
	/* Constructor/Destructor */
	CPDFiumPage(CPDFiumDoc* pDoc, int index);
	virtual ~CPDFiumPage();
	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pDoc->GetPDFiumPtr(); }

	/* Closure */
	//std::function<std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>& ()> GetPagePtr;
	//std::function<CSizeF()> GetSourceSize;
	UNQ_FPDF_PAGE& GetPagePtr() { return m_pPage; }
	CSizeF GetSourceSize() { return m_sourceSize; }
	/* Member function */
	void Render(const RenderEvent& e) { process_event(e); }
private:
	void Load();
	PdfBmpInfo GetLockBitmap()
	{
		std::lock_guard<std::mutex> lock(m_mtxBmp);
		return m_bmp;
	}
	void SetLockBitmap(const PdfBmpInfo& bmp)
	{
		std::lock_guard<std::mutex> lock(m_mtxBmp);
		m_bmp = bmp;
	}

	virtual void process_event(const RenderEvent& e);
	virtual void process_event(const LoadEvent& e);
	virtual void process_event(const ReloadEvent& e);
	virtual void process_event(const LoadCompletedEvent& e);
	virtual void process_event(const CancelCompletedEvent& e);
	virtual void process_event(const ErrorEvent& e);

	void None_Render(const RenderEvent& e);
	void Loading_OnEntry();
	//void Reloading_OnEntry(const ReloadEvent& e);
	void Loading_Render(const RenderEvent& e);
	void Available_Render(const RenderEvent& e);
	void WaitCancel_OnEntry();
	void WaitCancel_OnExit();
	void WaitCancel_Render(const RenderEvent& e);
	void Error_Render(const RenderEvent& e);


};

