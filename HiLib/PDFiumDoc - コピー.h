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

class CD2DWWindow;
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
	std::wstring m_password;
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
	std::function<int&()> GetPageCount;
	std::function<CSizeF&()> GetSourceSize;
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

class CPDFiumPage
{
private:
	/* Field */
	CPDFiumDoc* m_pDoc;
	int m_index;

	FLOAT m_loadingScale;

public:
	/* Constructor/Destructor */
	CPDFiumPage(CPDFiumDoc* pDoc, int index);
	virtual ~CPDFiumPage();
	/* Closure */
	std::function<std::shared_ptr<std::remove_pointer_t<FPDF_PAGE>>& ()> GetPagePtr;
	std::function<CSizeF()> GetSourceSize;
	std::function<CComPtr<ID2D1Bitmap>& (FLOAT req_scale)> GetBitmapPtr;
	/* Member function */	struct Machine;
	void Render(const RenderEvent& e);
};

