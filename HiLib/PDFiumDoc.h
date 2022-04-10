#pragma once

#include "Direct2DWrite.h"
#include "MyUniqueHandle.h"
#include <queue>
#include <future>
#include <boost/sml.hpp>

#include <fpdfview.h>
#pragma comment(lib, "pdfium.dll.lib")



struct fpdf_closedocument
{
	inline void operator()(FPDF_DOCUMENT pDoc){
		if(pDoc){
			FPDF_CloseDocument(pDoc);
		}
	}
};

struct fpdf_closepage
{
	void operator()(FPDF_PAGE pPage){
		if(pPage){
			FPDF_ClosePage(pPage);
		}
	}
};

struct fpdfbitmap_destroy {
  inline void operator()(FPDF_BITMAP pBitmap) 
  {
	  if (pBitmap) {
		  FPDFBitmap_Destroy(pBitmap);
	  }
  }
};


class CPDFiumPage;

class CPDFiumDoc
{
public:
	static void Init();
	static void Term();

private:
	CDirect2DWrite* m_pDirect;
	std::shared_ptr<FormatF> m_pFormat;
	std::wstring m_path;
	std::function<void()> m_changed;
public:
	CPDFiumDoc(
		const std::wstring& path,
		const std::wstring& password, 
		CDirect2DWrite* pDirect,
		const std::shared_ptr<FormatF>& pFormat,
		std::function<void()> changed);
	virtual ~CPDFiumDoc();

	CDirect2DWrite* GetDirectPtr() { return m_pDirect; }
	std::shared_ptr<FormatF> GetFormatPtr() const { return m_pFormat; }

	std::function<std::shared_ptr<std::remove_pointer_t<FPDF_DOCUMENT>>& ()> GetDocPtr;
	std::function<int()> GetPageCount;
	std::function<CSizeF()> GetSourceSize;
	std::function<std::vector<std::shared_ptr<CPDFiumPage>>& ()> GetPages;
	
	std::shared_ptr<CPDFiumPage>& GetPage(int index)
	{
		FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
		return GetPages()[index];
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

public:
	/* Constructor/Destructor */
	CPDFiumPage(CPDFiumDoc* pDoc, int index);
	virtual ~CPDFiumPage();
	/* Closure */
	std::function<std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>& ()> GetPagePtr;
	std::function<CSizeF()> GetSourceSize;
	/* Member function */	struct Machine;
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

