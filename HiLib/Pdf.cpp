#include "Pdf.h"
#include "Direct2DWrite.h"
#include "Debug.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

/********/
/* CPdf */
/********/

CPdf::CPdf(CDirect2DWrite* pDirect, const std::shared_ptr<FormatF>& pFormat)
	:m_pDirect(pDirect),
	m_pFormat(pFormat),
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
		m_future = std::async(std::launch::async, [this, doc, changed]()
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

/************/
/* CPdfPage */
/************/

struct CPdfPage::Machine
{
	//Status
	class None {};
	class Loading {};
	class Available {};
	class WaitCancel {};
	class Error {};

	template<class T, class R, class E>
	auto call(R(T::* f)(E))const
	{
		return [f](T* self, E e, boost::sml::back::process<E> process) { return (self->*f)(e); };
	}
	
	template<class T, class R>
	auto call(R(T::* f)())const
	{
		return [f](T* self) { return (self->*f)(); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<None> +event<RenderEvent> / call(&CPdfPage::None_Render),
			state<None> +event<LoadEvent> = state<Loading>,

			state<Loading> +on_entry<_> / call(&CPdfPage::Loading_OnEntry),

			state<Loading> +event<RenderEvent> / call(&CPdfPage::Loading_Render),
			state<Loading> +event<LoadCompletedEvent> = state<Available>,
			state<Loading> +event<ReloadEvent> = state<WaitCancel>,
			state<Loading> +event<ErrorEvent> = state<Error>,

			state<Available> +event<RenderEvent> / call(&CPdfPage::Available_Render),
			state<Available> +event<ReloadEvent> = state<Loading>,

			state<WaitCancel> +on_entry<_> / call(&CPdfPage::WaitCancel_OnEntry),
			state<WaitCancel> +on_exit<_> / call(&CPdfPage::WaitCancel_OnExit),

			state<WaitCancel> +event<RenderEvent> / call(&CPdfPage::WaitCancel_Render),
			state<WaitCancel> +event<LoadCompletedEvent> = state<Loading>,
			state<WaitCancel> +event<CancelCompletedEvent> = state<Loading>,

			state<Error> +event<RenderEvent> / call(&CPdfPage::Error_Render)

			//Error handler
			//*state<Error> +exception<std::exception> = state<Error>
		);
	}
};

void CPdfPage::process_event(const RenderEvent& e) { m_pMachine->process_event(e); }
void CPdfPage::process_event(const LoadEvent& e) { m_pMachine->process_event(e); }
void CPdfPage::process_event(const ReloadEvent& e) { m_pMachine->process_event(e); }
void CPdfPage::process_event(const LoadCompletedEvent& e) { m_pMachine->process_event(e); }
void CPdfPage::process_event(const CancelCompletedEvent& e) { m_pMachine->process_event(e); }
void CPdfPage::process_event(const ErrorEvent& e) { m_pMachine->process_event(e); }


CPdfPage::CPdfPage(CPdf* pPdf, UINT32 pageIndex )
	:m_pPdf(pPdf), m_pageIndex(pageIndex),
	m_spCancelThread(std::make_shared<bool>(false)),
	m_bmp{ CComPtr<IWICFormatConverter>(), 0.f },
	m_loadingScale(0.f), m_requestingScale(0.f),
	m_pMachine(new sml::sm<Machine, boost::sml::process_queue<std::queue>>{ this })
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
	SetLockBitmap({ CComPtr<IWICFormatConverter>(), 0.f });
}

/**************/
/* SML Action */
/**************/
void CPdfPage::None_Render(const RenderEvent& e)
{
	m_requestingScale = e.Scale;

	if (m_pPdf->GetDocument().second == PdfDocStatus::Available) {
		process_event(LoadEvent());
	} else {
		e.DirectPtr->DrawTextInRect(*m_pPdf->GetFormatPtr(), L"Loading Document...", e.RenderRectInWnd);
	}
}
void CPdfPage::Loading_OnEntry() 
{
	auto scale = m_requestingScale;

	if (m_pPdf->GetDocument().second == PdfDocStatus::Available) {
		m_loadingScale = scale;
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
		pOptions->put_DestinationWidth(static_cast<UINT32>(dstSize.width * scale));
		pOptions->put_DestinationHeight(static_cast<UINT32>(dstSize.height * scale));

		CComPtr<abifoundation::IAsyncAction> async;

		FAILED_THROW(pPage->RenderWithOptionsToStreamAsync(
			pMemStream,
			pOptions,
			&async));

		auto callback = wrl::Callback<abifoundation::IAsyncActionCompletedHandler>(
		[this, pMemStream, scale](_In_ abifoundation::IAsyncAction* asyncInfo, AsyncStatus status)
		{
			auto pbi = GetLockBitmap();
			if (*m_spCancelThread) {
				SetLockBitmap({ pbi.ConverterPtr, pbi.Scale });
				process_event(CancelCompletedEvent());
			} else if (status != AsyncStatus::Completed) {
				SetLockBitmap({ CComPtr<IWICFormatConverter>(),  0.f });
				process_event(ErrorEvent());
			} else {
				m_future = std::async(std::launch::async, [this, pMemStream, scale]()
				{
					auto pbi = GetLockBitmap();
					if (*m_spCancelThread) {
						SetLockBitmap({ pbi.ConverterPtr, pbi.Scale });
						process_event(CancelCompletedEvent());
					}else{
						CComPtr<IStream> pStream;
						FAILED_THROW(CreateStreamOverRandomAccessStream(pMemStream, IID_PPV_ARGS(&pStream)));
						CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
						FAILED_THROW(m_pPdf->GetDirectPtr()->GetWICImagingFactory()->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pWicBitmapDecoder));
						CComPtr<IWICBitmapFrameDecode> pWicBitmapFrameDecode;
						FAILED_THROW(pWicBitmapDecoder->GetFrame(0, &pWicBitmapFrameDecode));

						//Case of IWICBitmapSource
						//CComPtr<IWICBitmapSource> pWicBitmapSource;
						//WICPixelFormatGUID wicPixcelFormat = GUID_WICPixelFormat32bppPBGRA;
						//FAILED_THROW(WICConvertBitmapSource(wicPixcelFormat, pWicBitmapFrameDecode, &pWicBitmapSource));
						//SetLockBitmap(std::make_pair(pWicBitmapSource, PdfBmpStatus::Available));

						//Case of ID2D1Bitmap1
						//CComPtr<ID2D1Bitmap1> pBmp;
						//FAILED_THROW(m_pPdf->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWicBitmapSource, &pBmp));
						//SetLockBitmap(std::make_pair(pBmp, PdfBmpStatus::Available));
						//if (changed) { changed(); }


						//Case of IWICFormatConverter
						CComPtr<IWICFormatConverter> pConverter;
						FAILED_THROW(m_pPdf->GetDirectPtr()->GetWICImagingFactory()->CreateFormatConverter(&pConverter))
							FAILED_THROW(pConverter->Initialize(
								pWicBitmapFrameDecode,
								GUID_WICPixelFormat32bppPBGRA,
								WICBitmapDitherTypeNone,
								nullptr,
								0.0f,
								WICBitmapPaletteTypeCustom)
							);
						SetLockBitmap({ pConverter, scale });
						process_event(LoadCompletedEvent());

						//UINT imageWidth, imageHeight;
						//FAILED_THROW(pWicBitmapSource->GetSize(&imageWidth, &imageHeight));
					}
				});
			}
			return S_OK;
		});

		FAILED_THROW(async->put_Completed(callback.Get()));
	}

}

void CPdfPage::Loading_Render(const RenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();
	if (e.Scale != m_loadingScale) {
		process_event(ReloadEvent());
	}

	if (pbi.ConverterPtr) {
		CComPtr<ID2D1Effect> pBitmapEffect;
		FAILED_THROW(e.DirectPtr->GetD2DDeviceContext()->CreateEffect(CLSID_D2D1BitmapSource, &pBitmapEffect));

		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_INTERPOLATION_MODE, D2D1_BITMAPSOURCE_INTERPOLATION_MODE_MIPMAP_LINEAR);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ENABLE_DPI_CORRECTION, FALSE);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ALPHA_MODE, D2D1_BITMAPSOURCE_ALPHA_MODE_PREMULTIPLIED);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ORIENTATION, D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT);

		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(e.Scale/pbi.Scale, e.Scale/pbi.Scale));
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, pbi.ConverterPtr);

		e.DirectPtr->GetD2DDeviceContext()->DrawImage(pBitmapEffect, e.RenderRectInWnd.LeftTop());
	} else {
		e.DirectPtr->DrawTextInRect(*m_pPdf->GetFormatPtr(), L"Loading Page...", e.RenderRectInWnd);
	}
}

void CPdfPage::Available_Render(const RenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();
	if (e.Scale != pbi.Scale) {
		process_event(ReloadEvent());
	}

	if (pbi.ConverterPtr) {
		CComPtr<ID2D1Effect> pBitmapEffect;
		FAILED_THROW(e.DirectPtr->GetD2DDeviceContext()->CreateEffect(CLSID_D2D1BitmapSource, &pBitmapEffect));

		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_INTERPOLATION_MODE, D2D1_BITMAPSOURCE_INTERPOLATION_MODE_MIPMAP_LINEAR);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ENABLE_DPI_CORRECTION, FALSE);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ALPHA_MODE, D2D1_BITMAPSOURCE_ALPHA_MODE_PREMULTIPLIED);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ORIENTATION, D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT);

		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(e.Scale/pbi.Scale, e.Scale/pbi.Scale));
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, pbi.ConverterPtr);

		e.DirectPtr->GetD2DDeviceContext()->DrawImage(pBitmapEffect, e.RenderRectInWnd.LeftTop());
	}
}

void CPdfPage::WaitCancel_OnEntry()
{
	*m_spCancelThread = true;
}
void CPdfPage::WaitCancel_OnExit()
{
	*m_spCancelThread = false;
}

void CPdfPage::WaitCancel_Render(const RenderEvent& e) 
{
	m_requestingScale = e.Scale;

	auto pbi = GetLockBitmap();

	if (pbi.ConverterPtr) {
		CComPtr<ID2D1Effect> pBitmapEffect;
		FAILED_THROW(e.DirectPtr->GetD2DDeviceContext()->CreateEffect(CLSID_D2D1BitmapSource, &pBitmapEffect));

		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_INTERPOLATION_MODE, D2D1_BITMAPSOURCE_INTERPOLATION_MODE_MIPMAP_LINEAR);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ENABLE_DPI_CORRECTION, FALSE);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ALPHA_MODE, D2D1_BITMAPSOURCE_ALPHA_MODE_PREMULTIPLIED);
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_ORIENTATION, D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT);

		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(e.Scale/pbi.Scale, e.Scale/pbi.Scale));
		pBitmapEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, pbi.ConverterPtr);

		e.DirectPtr->GetD2DDeviceContext()->DrawImage(pBitmapEffect, e.RenderRectInWnd.LeftTop());
	} else {
		e.DirectPtr->DrawTextInRect(*m_pPdf->GetFormatPtr(), L"Loading Page...", e.RenderRectInWnd);
	}
}

void CPdfPage::Error_Render(const RenderEvent& e)
{
	m_requestingScale = e.Scale;

	e.DirectPtr->DrawTextInRect(*m_pPdf->GetFormatPtr(), L"Error on Page loading.", e.RenderRectInWnd);
}

