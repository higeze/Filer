#include "Pdf.h"
#include "Direct2DWrite.h"
#include "ThreadPool.h"
#include "Debug.h"

CPdfPage::CPdfPage(CDirect2DWrite* pDirect, CComPtr<abipdf::IPdfDocument> pDoc, UINT32 pageIndex )
	:m_pDirect(pDirect), m_pDoc(pDoc), m_pageIndex(pageIndex), m_spCancelThread(std::make_shared<bool>(false))
{
	GetSourceSize = [sz = CSizeF(), this]()mutable->CSizeF{
			
		if (sz.width == 0 || sz.height == 0) {
			CComPtr<abipdf::IPdfPage> pPage;
			m_pDoc->GetPage(m_pageIndex, &pPage);
			ABI::Windows::Foundation::Size size;
			pPage->get_Size(&size);
			sz.width = size.Width;
			sz.height = size.Height;
		}
		return sz;
	};
}

CPdfPage::~CPdfPage()
{
	*m_spCancelThread = true;
}

void CPdfPage::LoadBitmap(std::function<void()> changed)
{
	CComPtr<abipdf::IPdfPage> pPage;
	FAILED_THROW(m_pDoc->GetPage(m_pageIndex, &pPage));

	abifoundation::Size pdfPageSize;
	FAILED_THROW(pPage->get_Size(&pdfPageSize));

	CSizeF srcSize(pdfPageSize.Width, pdfPageSize.Height);
	CSizeF dstSize(srcSize);

	CComPtr<abistream::IRandomAccessStream> pMemStream;
	wrlwrappers::HStringReference inmemoryname(RuntimeClass_Windows_Storage_Streams_InMemoryRandomAccessStream);
	FAILED_THROW(winfoundation::ActivateInstance(inmemoryname.Get(), &pMemStream));

	CComPtr<abipdf::IPdfPageRenderOptions> pOptions;
	wrlwrappers::HStringReference optionsname(RuntimeClass_Windows_Data_Pdf_PdfPageRenderOptions);
	FAILED_THROW(winfoundation::ActivateInstance(optionsname.Get(), &pOptions));

	pOptions->put_SourceRect(ABI::Windows::Foundation::Rect{ 0.f, 0.f, srcSize.width, srcSize.height });
	pOptions->put_DestinationWidth(static_cast<UINT32>(dstSize.width));
	pOptions->put_DestinationHeight(static_cast<UINT32>(dstSize.height));

	CComPtr<abifoundation::IAsyncAction> async;

	FAILED_THROW(pPage->RenderWithOptionsToStreamAsync(
		pMemStream,
		pOptions,
		&async));

	auto callback = wrl::Callback<abifoundation::IAsyncActionCompletedHandler>(
	[this, pMemStream, changed](_In_ abifoundation::IAsyncAction* asyncInfo, AsyncStatus status)
	{
		if (*m_spCancelThread) { return S_OK; }
		CThreadPool::GetInstance()->enqueue([this, pMemStream, changed]()
		{
			CComPtr<IStream> pStream;
			FAILED_THROW(CreateStreamOverRandomAccessStream(pMemStream, IID_PPV_ARGS(&pStream)));
			CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
			FAILED_THROW(m_pDirect->GetWICImagingFactory()->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pWicBitmapDecoder));
			CComPtr<IWICBitmapFrameDecode> pWicBitmapFrameDecode;
			FAILED_THROW(pWicBitmapDecoder->GetFrame(0, &pWicBitmapFrameDecode));
			CComPtr<IWICBitmapSource> pWicBitmapSource;
			WICPixelFormatGUID wicPixcelFormat = GUID_WICPixelFormat32bppPBGRA;
			FAILED_THROW(WICConvertBitmapSource(wicPixcelFormat, pWicBitmapFrameDecode, &pWicBitmapSource));
			UINT imageWidth, imageHeight;
			FAILED_THROW(pWicBitmapSource->GetSize(&imageWidth, &imageHeight));
			CComPtr<ID2D1Bitmap1> pBmp;
			FAILED_THROW(m_pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWicBitmapSource, &pBmp));
			SetLockBitmap(std::make_pair(pBmp, PdfBmpStatus::Available));
			if (changed) { changed(); }
		});
		return S_OK;
	});

	FAILED_THROW(async->put_Completed(callback.Get()));
}

std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus> CPdfPage::GetBitmap(std::function<void()> changed)
{

	switch (GetLockBitmap().second) {
		case PdfBmpStatus::None:
			SetLockBitmap(std::make_pair(CComPtr<ID2D1Bitmap1>(), PdfBmpStatus::Loading));
			LoadBitmap(changed);
			break;
		case PdfBmpStatus::Loading:
		case PdfBmpStatus::Available:
		case PdfBmpStatus::Unavailable:
			break;
	}
	return GetLockBitmap();
}