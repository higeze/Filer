#pragma once
#include "ReactiveProperty.h"
#include "Debug.h"
#include "atlcomcli.h"
#include <msctf.h>
#include <cstdlib>
#include <utility>
 
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
   
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "windows.data.pdf.lib")
#pragma comment(lib, "windowscodecs.lib")

//TODOHIGH
namespace abipdf = ABI::Windows::Data::Pdf;
namespace abifoundation = ABI::Windows::Foundation;
namespace abistorage = ABI::Windows::Storage;
namespace abistreams = ABI::Windows::Storage::Streams;
namespace winfoundation = Windows::Foundation;
namespace wrl = Microsoft::WRL;
namespace wrlwrappers = Microsoft::WRL::Wrappers;

#include <future>
#include <mutex>

class CVScroll;
class CHScroll;
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
	std::pair<CComPtr<abipdf::IPdfDocument>, PdfDocStatus>  m_document;
	std::mutex m_mtxDoc;
	std::shared_ptr<bool> m_spCancelThread;
	std::future<void> m_future;
	std::vector<std::unique_ptr<CPdfPage>> m_pages;
public:
	CPdf(CDirect2DWrite* pDirect);
	virtual ~CPdf();

	CDirect2DWrite* GetDirectPtr() { return m_pDirect; }
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
	void SetLockDocument(std::pair<CComPtr<abipdf::IPdfDocument>, PdfDocStatus>& document)
	{
		std::lock_guard<std::mutex> lock(m_mtxDoc);
		m_document = document;
	}


};

enum class PdfBmpStatus
{
	None,
	Available,
	Loading,
	Unavailable,
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
	std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus> m_bmp;
public:
	/* Constructor/Destructor */
	CPdfPage(CPdf* pDoc, UINT32 pageIndex);
	virtual ~CPdfPage();
	/* Closure */
	std::function<CSizeF&()> GetSourceSize;
	/* Member function */
	std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus> GetBitmap(std::function<void()> changed);

private:
	void Load(std::function<void()> changed);
	void Clear();
	std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus> GetLockBitmap()
	{
		std::lock_guard<std::mutex> lock(m_mtxBmp);
		return m_bmp;
	}
	void SetLockBitmap(std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus>& bmp)
	{
		std::lock_guard<std::mutex> lock(m_mtxBmp);
		m_bmp = bmp;
	}
};
