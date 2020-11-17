#include "Pdf.h"
#include "Direct2DWrite.h"
#include "ThreadPool.h"
#include "Debug.h"

CPdf::CPdf(CDirect2DWrite* pDirect)
	:m_pDirect(pDirect),
	m_document(std::make_pair(CComPtr<abipdf::IPdfDocument>(), PdfDocStatus::None)),
		m_spCancelThread(std::make_shared<bool>(false))
{
	GetSourceSize = [sz = CSizeF(), this]()mutable->CSizeF&
	{		
		if (GetDocument().second == PdfDocStatus::Available && (sz.width == 0 || sz.height == 0)) {
			for(const auto& pPage : m_pages){
				sz.width = (std::max)(sz.width, pPage->GetSourceSize().width);
				sz.height += pPage->GetSourceSize().height;
			}
		}
		return sz;
	};
}

CPdf::~CPdf()
{
	*m_spCancelThread = true;
}

void CPdf::Clear()
{
	GetSourceSize() = CSizeF();
	SetLockDocument(std::make_pair(CComPtr<abipdf::IPdfDocument>(), PdfDocStatus::None));
	m_pages.clear();
}

void CPdf::Load(const std::wstring& path, std::function<void()> changed)
{
	Clear();

	CComPtr<abistreams::IRandomAccessStream> s;
	FAILED_THROW(::CreateRandomAccessStreamOnFile(path.c_str(), abistorage::FileAccessMode_Read, IID_PPV_ARGS(&s)));
	
	CComPtr<abipdf::IPdfDocumentStatics> pdfDocumentsStatics;
	wrlwrappers::HStringReference className(RuntimeClass_Windows_Data_Pdf_PdfDocument);
	// à»â∫ÇÕRoGetActivationFactoryÇÃÉâÉbÉpÅ[
	FAILED_THROW(winfoundation::GetActivationFactory(className.Get(), &pdfDocumentsStatics));
	CComPtr<abifoundation::IAsyncOperation<abipdf::PdfDocument*>> async;
	FAILED_THROW(pdfDocumentsStatics->LoadFromStreamAsync(s, &async));
	auto callback = wrl::Callback<abifoundation::IAsyncOperationCompletedHandler<abipdf::PdfDocument*>>(
	[this, changed](_In_ abifoundation::IAsyncOperation<abipdf::PdfDocument*>* async, AsyncStatus status)
	{
		if (*m_spCancelThread || status != AsyncStatus::Completed) { return S_OK; }
		CComPtr<abipdf::IPdfDocument> doc;
		FAILED_THROW(async->GetResults(&doc));
		m_future = CThreadPool::GetInstance()->enqueue([this, doc, changed]()
		{
			SetLockDocument(std::make_pair(std::move(doc), PdfDocStatus::Available));
			UINT32 count;
			GetLockDocument().first->get_PageCount(&count);
			for (UINT32 i = 0; i < count; i++) {
				m_pages.push_back(std::make_unique<CPdfPage>(this, i));
			}
			changed();
		});
		return S_OK;
	});
	FAILED_THROW(async->put_Completed(callback.Get()));
}

CPdfPage::CPdfPage(CPdf* pPdf, UINT32 pageIndex )
	:m_pPdf(pPdf), m_pageIndex(pageIndex), m_spCancelThread(std::make_shared<bool>(false))
{
	GetSourceSize = [sz = CSizeF(), this]()mutable->CSizeF&
	{		
		if (m_pPdf->GetDocument().second == PdfDocStatus::Available && (sz.width == 0 || sz.height == 0)) {
			CComPtr<abipdf::IPdfPage> pPage;
			m_pPdf->GetDocument().first->GetPage(m_pageIndex, &pPage);
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

void CPdfPage::Clear()
{
	GetSourceSize() = CSizeF();
	SetLockBitmap(std::make_pair(CComPtr<ID2D1Bitmap1>(), PdfBmpStatus::None));
}


void CPdfPage::Load(std::function<void()> changed)
{
	if (m_pPdf->GetDocument().second == PdfDocStatus::Available) {
		CComPtr<abipdf::IPdfPage> pPage;
		FAILED_THROW(m_pPdf->GetDocument().first->GetPage(m_pageIndex, &pPage));

		abifoundation::Size pdfPageSize;
		FAILED_THROW(pPage->get_Size(&pdfPageSize));

		CSizeF srcSize(pdfPageSize.Width, pdfPageSize.Height);
		CSizeF dstSize(srcSize);

		CComPtr<abistreams::IRandomAccessStream> pMemStream;
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
			if (*m_spCancelThread  || status != AsyncStatus::Completed) { 
				SetLockBitmap(std::make_pair(CComPtr<ID2D1Bitmap1>(), PdfBmpStatus::Unavailable));
			} else {
				m_future = CThreadPool::GetInstance()->enqueue([this, pMemStream, changed]()
				{
						CComPtr<IStream> pStream;
						FAILED_THROW(CreateStreamOverRandomAccessStream(pMemStream, IID_PPV_ARGS(&pStream)));
						CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
						FAILED_THROW(m_pPdf->GetDirectPtr()->GetWICImagingFactory()->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pWicBitmapDecoder));
						CComPtr<IWICBitmapFrameDecode> pWicBitmapFrameDecode;
						FAILED_THROW(pWicBitmapDecoder->GetFrame(0, &pWicBitmapFrameDecode));
						CComPtr<IWICBitmapSource> pWicBitmapSource;
						WICPixelFormatGUID wicPixcelFormat = GUID_WICPixelFormat32bppPBGRA;
						FAILED_THROW(WICConvertBitmapSource(wicPixcelFormat, pWicBitmapFrameDecode, &pWicBitmapSource));
						UINT imageWidth, imageHeight;
						FAILED_THROW(pWicBitmapSource->GetSize(&imageWidth, &imageHeight));
						CComPtr<ID2D1Bitmap1> pBmp;
						FAILED_THROW(m_pPdf->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWicBitmapSource, &pBmp));
						SetLockBitmap(std::make_pair(pBmp, PdfBmpStatus::Available));
						if (changed) { changed(); }
				});
			}
			return S_OK;
		});

		FAILED_THROW(async->put_Completed(callback.Get()));
	}
}

std::pair<CComPtr<ID2D1Bitmap1>, PdfBmpStatus> CPdfPage::GetBitmap(std::function<void()> changed)
{

	switch (GetLockBitmap().second) {
		case PdfBmpStatus::None:
			SetLockBitmap(std::make_pair(CComPtr<ID2D1Bitmap1>(), PdfBmpStatus::Loading));
			Load(changed);
			break;
		case PdfBmpStatus::Loading:
		case PdfBmpStatus::Available:
		case PdfBmpStatus::Unavailable:
			break;
	}
	return GetLockBitmap();
}