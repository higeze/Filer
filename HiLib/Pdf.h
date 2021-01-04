#pragma once
#include "Direct2DWrite.h"
#include "ReactiveProperty.h"
#include "Debug.h"
#include "atlcomcli.h"
#include <msctf.h>
#include <cstdlib>
#include <utility>
#include <queue>
 
#include <windows.h>
#include <shcore.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <d2d1_2.h>
#include <wincodec.h>

#include <windows.storage.h>
#include <windows.storage.streams.h>
#include <windows.data.pdf.h>
#include <windows.data.pdf.interop.h>

#include <wrl/client.h>
#include <wrl/event.h>
#include <boost/sml.hpp>
   
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "windows.data.pdf.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace abipdf = ABI::Windows::Data::Pdf;
namespace abifoundation = ABI::Windows::Foundation;
namespace abistorage = ABI::Windows::Storage;
namespace abistreams = ABI::Windows::Storage::Streams;
namespace winfoundation = Windows::Foundation;
namespace wrl = Microsoft::WRL;
namespace wrlwrappers = Microsoft::WRL::Wrappers;

#include <future>
#include <mutex>

class CDirect2DWrite;
class CPdf;
class CPdfPage;
struct CSizeF;

enum class PdfDocStatus
{
	None,
	Available,
	Loading,
	Unavailable,
};


class CPdf
{
private:
	CDirect2DWrite* m_pDirect;
	std::shared_ptr<FormatF> m_pFormat;
	std::pair<CComPtr<abipdf::IPdfDocument>, PdfDocStatus>  m_document;
	std::mutex m_mtxDoc;
	std::shared_ptr<bool> m_spCancelThread;
	std::future<void> m_future;
	std::vector<std::unique_ptr<CPdfPage>> m_pages;
public:
	CPdf(CDirect2DWrite* pDirect, const std::shared_ptr<FormatF>& pFormat);
	virtual ~CPdf();

	CDirect2DWrite* GetDirectPtr() { return m_pDirect; }
	std::shared_ptr<FormatF> GetFormatPtr() const { return m_pFormat; }
	std::function<CSizeF&()> GetSourceSize;

	void Load(const std::wstring& path, std::function<void()> changed);
	void Clear();
	std::pair<CComPtr<abipdf::IPdfDocument>, PdfDocStatus> GetDocument()
	{
		return GetLockDocument();
	}

	UINT32 GetPageCount()const { return m_pages.size(); }
	std::unique_ptr<CPdfPage>& GetPage(UINT32 index)
	{
		FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
		return m_pages[index];
	}
private:
	std::pair<CComPtr<abipdf::IPdfDocument>, PdfDocStatus> GetLockDocument()
	{
		std::lock_guard<std::mutex> lock(m_mtxDoc);
		return m_document;
	}
	void SetLockDocument(const std::pair<CComPtr<abipdf::IPdfDocument>, PdfDocStatus>& document)
	{
		std::lock_guard<std::mutex> lock(m_mtxDoc);
		m_document = document;
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
	CComPtr<IWICFormatConverter> ConverterPtr;
	FLOAT Scale;
};

class CPdfPage
{
private:
	/* Field */
	CPdf* m_pPdf;
	UINT32 m_pageIndex;
	std::mutex m_mtxBmp;
	std::shared_ptr<bool> m_spCancelThread;
	std::future<void> m_future;
	PdfBmpInfo m_bmp;
	FLOAT m_requestingScale;
	FLOAT m_loadingScale;
	struct Machine;
	std::unique_ptr<boost::sml::sm<Machine, boost::sml::process_queue<std::queue>>> m_pMachine;
	std::function<void()> StateChanged;

public:
	/* Constructor/Destructor */
	CPdfPage(CPdf* pDoc, UINT32 pageIndex);
	virtual ~CPdfPage();
	/* Closure */
	std::function<CSizeF&()> GetSourceSize;
	/* Member function */
	//PdfBmpInfo GetBitmap(FLOAT scale, std::function<void()> changed);
	void Render(const RenderEvent& e) { process_event(e); }
private:
	void Clear();
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
