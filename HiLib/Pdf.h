#pragma once
#include "ReactiveProperty.h"
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
namespace abistream = ABI::Windows::Storage::Streams;
namespace winfoundation = Windows::Foundation;
namespace wrl = Microsoft::WRL;
namespace wrlwrappers = Microsoft::WRL::Wrappers;

#include <future>
#include <mutex>

class CVScroll;
class CHScroll;
class CDirect2DWrite;
struct CSizeF;

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
	CDirect2DWrite* m_pDirect;
	CComPtr<abipdf::IPdfDocument> m_pDoc;
	UINT32 m_pageIndex;
	std::mutex m_mtxBmp;
	std::shared_ptr<bool> m_spCancelThread;
	std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus> m_bmp;
public:
	/* Constructor/Destructor */
	CPdfPage(CDirect2DWrite* pDirect, CComPtr<abipdf::IPdfDocument> pDoc, UINT32 pageIndex);
	virtual ~CPdfPage();
	/* Closure */
	std::function<CSizeF()> GetSourceSize;
	/* Member function */
	std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus> GetBitmap(std::function<void()> changed);

private:
	void LoadBitmap(std::function<void()> changed);
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
