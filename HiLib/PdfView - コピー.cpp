#include "PdfView.h"
#include "D2DWWindow.h"
#include "Scroll.h"
#include "Debug.h"
#include <functional>

CPdfView::CPdfView(CD2DWControl* pParentControl, const std::shared_ptr<PdfViewProperty>& pProp)
	:CD2DWControl(pParentControl),
    m_pProp(pProp),
	m_pdfDocument(nullptr),
	m_pMachine(std::make_unique<CPdfViewStateMachine>(this)),
	m_pVScroll(std::make_unique<CVScroll>(this, pProp->VScrollPropPtr, [this](const wchar_t* name) { })),
	m_pHScroll(std::make_unique<CHScroll>(this, pProp->HScrollPropPtr, [this](const wchar_t* name) { }))
{}

CPdfView::~CPdfView() = default;

void CPdfView::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);

	m_pDirect = std::make_unique<CDirect2DWrite>(GetWndPtr()->m_hWnd);

	GetPdfRenderer = [p = CComPtr<IPdfRendererNative>(), this]() mutable->CComPtr<IPdfRendererNative>&
	{
		if (!p) {
			::PdfCreateRenderer(m_pDirect->GetDXGIDevice(), &p);
		}
		return p;
	};

    GetRenderRectInWnd = [rc = CRectF(), this]() mutable ->CRectF&
    {
        rc = GetRectInWnd();
	    rc.DeflateRect(m_pProp->FocusedLine->Width * 0.5f);
	    rc.DeflateRect(*(m_pProp->Padding));
	    return rc;
    };

	GetRenderSize = [sz = CSizeF(), this]() mutable->CSizeF&
	{
		sz = GetRenderRectInWnd().Size();
		return sz;
	};


    GetRealContentSize = [sz = CSizeF(), this]() mutable->CSizeF&
    {
        sz = CSizeF();
        if (!m_pdfDocument) { return sz; }

        CSizeF lastSize;
        for(size_t i = 0; i<m_pdfPages.size(); i++){
            if (i == 0 && !m_pdfPages[i]) {
                if (FAILED(m_pdfDocument->GetPage(i, &m_pdfPages[i]))) {
                    throw std::exception(FILE_LINE_FUNC);
                }
            }

            ABI::Windows::Foundation::Size pdfPageSize;
            if (!m_pdfPages[i]) {
                pdfPageSize.Width = lastSize.width;
                pdfPageSize.Height = lastSize.height;
            } else {
                if (FAILED(m_pdfPages[i]->get_Size(&pdfPageSize))) {
                    throw std::exception(FILE_LINE_FUNC);
                }            
            }

            lastSize.width = pdfPageSize.Width;
            lastSize.height = pdfPageSize.Height;
            sz.width = (std::max)(sz.width, pdfPageSize.Width);
            sz.height += pdfPageSize.Height;
        }
        return sz;
    };

    GetRenderContentSize = [sz = CSizeF(), this]() mutable->CSizeF&
    {
        sz = GetRealContentSize();
        sz.width *= m_scale;
        sz.height *= m_scale;
		return sz;
    };
}

void CPdfView::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CPdfView::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
}

void CPdfView::OnMouseWheel(const MouseWheelEvent& e)
{
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollDelta() * e.Delta / WHEEL_DELTA);
}

void CPdfView::Normal_Paint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetRectInWnd());
	//Paint Focused Line
	if (GetIsFocused() ){
		CRectF rcFocus(GetRectInWnd());
		rcFocus.DeflateRect(1.0f, 1.0f);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangle(*(m_pProp->FocusedLine), rcFocus);
	}
	//PaintContent

	//if (m_pdfDocument != nullptr) {
	//	CPointF lefttopInWnd = GetRenderRectInWnd().LeftTop();
	//	CPointF lefttopInRender = CPointF(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());

	//	//m_pdfPages.clear();
	//	//UINT count;
	//	//m_pdfDocument->get_PageCount(&count);
	//	//m_pdfPages.assign(count, nullptr);

	//	for (size_t pageNum = 0; pageNum < m_pdfPages.size(); pageNum++) {
	//		if (m_pdfPages[pageNum] == nullptr) {
	//			if (FAILED(m_pdfDocument->GetPage(pageNum, &m_pdfPages[pageNum]))) {
	//				throw std::exception(FILE_LINE_FUNC);
	//			}
	//		}

	//		ABI::Windows::Foundation::Size pdfPageSize;
	//		m_pdfPages[pageNum]->get_Size(&pdfPageSize);

	//		PDF_RENDER_PARAMS params = PdfRenderParams(
	//			D2D1::RectF(0.f, 0.f, pdfPageSize.Width, pdfPageSize.Height),
	//			pdfPageSize.Width * m_scale,
	//			pdfPageSize.Height * m_scale);

	//		if ((lefttopInRender.y >= 0 && lefttopInRender.y <= GetRenderSize().height) ||
	//		   (lefttopInRender.y + params.DestinationHeight >= 0 && lefttopInRender.y + params.DestinationHeight <= GetRenderSize().height)) {
	//			D2D_MATRIX_3X2_F mat{ 1.f, 0, 0, 1.f,  lefttopInWnd.x + lefttopInRender.x, lefttopInWnd.y + lefttopInRender.y };
	//			GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetTransform(mat);
	//			GetPdfRenderer()->RenderPageToDeviceContext(
	//				m_pdfPages[pageNum], GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext(), &params);
	//			GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetTransform(D2D1::Matrix3x2F::Identity());
	//		}

	//		lefttopInRender.y += params.DestinationHeight;
	//		if (lefttopInRender.y > GetRenderSize().height) {
	//			break;
	//		}
	//	}
	//}

        CPointF lefttopInWnd = GetRenderRectInWnd().LeftTop();
		CPointF lefttopInRender = CPointF(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());

		m_pdfPages.clear();
		UINT count;
		m_pdfDocument->get_PageCount(&count);
		m_pdfPages.assign(count, nullptr);
		

		for (size_t pageNum = 0; pageNum < m_pdfPages.size(); pageNum++) {
            if (m_pdfPages[pageNum] == nullptr) {
				if (FAILED(m_pdfDocument->GetPage(pageNum, &m_pdfPages[pageNum]))) {
					throw std::exception(FILE_LINE_FUNC);
				}
            }

            ABI::Windows::Foundation::Size pdfPageSize;
            m_pdfPages[pageNum]->get_Size(&pdfPageSize);
        
            PDF_RENDER_PARAMS params = PdfRenderParams(
                D2D1::RectF(0.f, 0.f, pdfPageSize.Width, pdfPageSize.Height),
                pdfPageSize.Width * m_scale,
                pdfPageSize.Height * m_scale);

			if ((lefttopInRender.y >= 0 && lefttopInRender.y <= GetRenderSize().height) ||
			   (lefttopInRender.y + params.DestinationHeight >= 0 && lefttopInRender.y + params.DestinationHeight <= GetRenderSize().height)) {

				if (m_pdfBmps[pageNum] == nullptr) {
					//m_pDirect->Resize();
					CComPtr<ID2D1Bitmap> pBitmap;

					// Get screen DPI
					FLOAT dpiX, dpiY;
					dpiX = dpiY = static_cast<FLOAT>(::GetDpiForWindow(GetWndPtr()->m_hWnd));

					// Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
					D2D1_BITMAP_PROPERTIES bitmapProperties =
						D2D1::BitmapProperties(
						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);
					m_pDirect->GetD2DDeviceContext()->CreateBitmap(
						D2D1_SIZE_U{ 200, 200 }, bitmapProperties, &pBitmap);
					m_pDirect->GetD2DDeviceContext()->SetTarget(pBitmap);

					m_pDirect->GetD2DDeviceContext()->BeginDraw();
//					m_pDirect->ClearSolid(CColorF(1.f, 1.f, 1.f));

					GetPdfRenderer()->RenderPageToDeviceContext(m_pdfPages[pageNum], m_pDirect->GetD2DDeviceContext(), &params);
					m_pDirect->GetD2DDeviceContext()->EndDraw();
					//m_pdfBmps[pageNum] = m_pDirect->GetD2DBackBuffer();
					m_pdfBmps[pageNum] = pBitmap;
				//	CComPtr<IRandomAccessStream> pMemStream;
				//	Microsoft::WRL::Wrappers::HStringReference inmemoryname(RuntimeClass_Windows_Storage_Streams_InMemoryRandomAccessStream);
				//	Windows::Foundation::ActivateInstance(inmemoryname.Get(), &pMemStream);

				//	CComPtr<IPdfPageRenderOptions> pOptions;
				//	Microsoft::WRL::Wrappers::HStringReference optionsname(RuntimeClass_Windows_Data_Pdf_PdfPageRenderOptions);
				//	Windows::Foundation::ActivateInstance(optionsname.Get(), &pOptions);
				//	pOptions->put_SourceRect(ABI::Windows::Foundation::Rect{ 0.f, 0.f, pdfPageSize.Width, pdfPageSize.Height });
				//	pOptions->put_DestinationWidth(pdfPageSize.Width * m_scale);
				//	pOptions->put_DestinationHeight(pdfPageSize.Height * m_scale);

				//	CComPtr<IAsyncAction> async;

				//	m_pdfPages[pageNum]->RenderWithOptionsToStreamAsync(
				//		pMemStream,
				//		pOptions,
				//		&async);

				//	auto callback = Callback<IAsyncActionCompletedHandler>(
				//	[this, p = pMemStream, n = pageNum](_In_ IAsyncAction* asyncInfo, AsyncStatus status)
				//	{
				//		HRESULT hr;
				//			CComPtr<IStream> pStream;
				//			hr = CreateStreamOverRandomAccessStream(p, IID_PPV_ARGS(&pStream));
				//			CComPtr<IWICBitmapEncoder> pWicBitmapEncoder;
				//			hr = GetWndPtr()->GetDirectPtr()->GetWICImagingFactory()->CreateEncoder(GUID_ContainerFormatBmp, nullptr, &pWicBitmapEncoder);
				//			hr = pWicBitmapEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
				//			CComPtr<IWICBitmapFrameEncode> pWicBitmapFrameEncode;
				//			hr = pWicBitmapEncoder->CreateNewFrame(&pWicBitmapFrameEncode, nullptr);
				//			hr = pWicBitmapFrameEncode->Initialize(nullptr);
				//			CComPtr<IWICImageEncoder> pWicImageEncoder;
				//			hr = GetWndPtr()->GetDirectPtr()->GetWICImagingFactory()->CreateImageEncoder(GetWndPtr()->GetDirectPtr()->GetD2DDevice(), &pWicImageEncoder);
				//			CComPtr<ID2D1Bitmap> pBitmap;
				//			IDXGISurface* dxgiBackBuffer;

				//// Get screen DPI
				//FLOAT dpiX, dpiY;
				//dpiX = dpiY = static_cast<FLOAT>(::GetDpiForWindow(GetWndPtr()->m_hWnd));

				//// Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
				//D2D1_BITMAP_PROPERTIES bitmapProperties =
				//	D2D1::BitmapProperties(
				//	D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);
				//hr = GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmap(
				//	D2D1_SIZE_U{ 200, 200 }, bitmapProperties, &pBitmap);
				//			
				////hr = GetWndPtr()->GetDirectPtr()->GetDXGISwapChain()->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
				////// Get screen DPI
				////FLOAT dpiX, dpiY;
				////dpiX = dpiY = static_cast<FLOAT>(::GetDpiForWindow(GetWndPtr()->m_hWnd));

				////// Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
				////D2D1_BITMAP_PROPERTIES1 bitmapProperties =
				////	D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				////	D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);

				////hr = GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &bitmapProperties, &pBitmap);
				//			hr = pWicImageEncoder->WriteFrame(pBitmap, pWicBitmapFrameEncode, nullptr);
				//			hr = pWicBitmapFrameEncode->Commit();
				//			hr = pWicBitmapEncoder->Commit();
				//			hr = pStream->Commit(STGC_DEFAULT);
				//			m_pdfBmps[n] = pBitmap;

				//			return 0;
				//	});
				//	async->put_Completed(callback.Get());
				} else {
					//D2D_MATRIX_3X2_F mat{ 1.f, 0, 0, 1.f,  lefttopInWnd.x + lefttopInRender.x, lefttopInWnd.y + lefttopInRender.y };
					//GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetTransform(mat);
					//GetWndPtr()->GetDirectPtr()->DrawBitmap(m_pdfBmps[pageNum], GetRenderRectInWnd());
					//GetPdfRenderer()->RenderPageToDeviceContext(
					//	m_pdfPages[pageNum], GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext(), &params);
					//GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetTransform(D2D1::Matrix3x2F::Identity());
				}
			}

 			lefttopInRender.y += params.DestinationHeight;
			if (lefttopInRender.y > GetRenderSize().height) {
				break;
			}
        }

    //}

	//PaintScroll
	UpdateScroll();
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CPdfView::Normal_KeyDown(const KeyDownEvent& e)
{
	//bool ctrl = ::GetAsyncKeyState(VK_CONTROL);
	//switch (e.Char) {
	//	case 'O':
	//		if (ctrl) {
	//			Open();
	//		}
	//		break;
	//	default:
	//		CD2DWControl::OnKeyDown(e);
	//		break;
	//}
}

/***************/
/* VScrollDrag */
/***************/
void CPdfView::VScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_pVScroll->SetState(UIElementState::Dragged);
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
void CPdfView::VScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_pVScroll->SetState(UIElementState::Normal);
	m_pVScroll->SetStartDrag(0.f);
}
bool CPdfView::VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pVScroll->GetIsVisible() && m_pVScroll->GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}
void CPdfView::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pVScroll->SetScrollPos(
		m_pVScroll->GetScrollPos() +
		(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y) - m_pVScroll->GetStartDrag()) *
		m_pVScroll->GetScrollDistance() /
		m_pVScroll->GetRectInWnd().Height());
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}


/***************/
/* HScrollDrag */
/***************/
void CPdfView::HScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_pHScroll->SetState(UIElementState::Dragged);
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CPdfView::HScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_pHScroll->SetState(UIElementState::Normal);
	m_pHScroll->SetStartDrag(0.f);
}

bool CPdfView::HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pHScroll->GetIsVisible() && m_pHScroll->GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}

void CPdfView::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pHScroll->SetScrollPos(
		m_pHScroll->GetScrollPos() +
		(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x) - m_pHScroll->GetStartDrag()) *
		m_pHScroll->GetScrollDistance() /
		m_pHScroll->GetRectInWnd().Width());
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CPdfView::Error_StdException(const std::exception& e)
{
	::OutputDebugStringA(e.what());

	std::string msg = fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());

	MessageBoxA(GetWndPtr()->m_hWnd, msg.c_str(), "Exception in StateMachine", MB_ICONWARNING);
}

void CPdfView::Open(const std::wstring& path)
{
	m_pdfPages.clear();

	CComPtr<IRandomAccessStream> s;
	auto hr = CreateRandomAccessStreamOnFile(
	  L"C:\\Users\\kuuna\\Desktop\\Rename32\\etc_191001-1j.pdf", FileAccessMode_Read, IID_PPV_ARGS(&s));
	if (FAILED(hr))return;
	
	CComPtr<IPdfDocumentStatics> pdfDocumentsStatics;
	Microsoft::WRL::Wrappers::HStringReference className(RuntimeClass_Windows_Data_Pdf_PdfDocument);
	// à»â∫ÇÕRoGetActivationFactoryÇÃÉâÉbÉpÅ[
	hr = Windows::Foundation::GetActivationFactory(
	  className.Get(), &pdfDocumentsStatics);
	if (FAILED(hr))return;
	CComPtr<IAsyncOperation<PdfDocument*>> async;
	hr = pdfDocumentsStatics->LoadFromStreamAsync(s, &async);
	if (FAILED(hr))return;
	auto callback = Callback<IAsyncOperationCompletedHandler<PdfDocument*>>(
	  [this](_In_ IAsyncOperation<PdfDocument*>* async, AsyncStatus status)
	{
		if (status != AsyncStatus::Completed)
			return S_FALSE;
		CComPtr<IPdfDocument> doc;
		auto hr = async->GetResults(&doc);
		if (SUCCEEDED(hr)) {
			m_pdfDocument = std::move(doc);
			UINT count;
			m_pdfDocument->get_PageCount(&count);
			m_pdfPages.assign(count, nullptr);
			m_pdfBmps.assign(count, nullptr);

			if (FAILED(m_pdfDocument->GetPage(0, &m_pdfPages[0]))) {
				throw std::exception(FILE_LINE_FUNC);
			}

            ABI::Windows::Foundation::Size pdfPageSize;
            m_pdfPages[0]->get_Size(&pdfPageSize);

			FLOAT scaleX = GetRenderSize().width / pdfPageSize.Width;
            FLOAT scaleY = GetRenderSize().height / pdfPageSize.Height;
            m_scale = (std::min)(scaleX, scaleY);

			GetWndPtr()->InvalidateRect(NULL, FALSE);
		}
		return hr;
	});
	hr = async->put_Completed(callback.Get());
	if (FAILED(hr))return;
}

void CPdfView::UpdateScroll()
{
	//VScroll
	//Page
	m_pVScroll->SetScrollPage(GetRenderSize().height);
	//Range
	m_pVScroll->SetScrollRange(0, GetRenderContentSize().height);

	//HScroll
	//Page
	m_pHScroll->SetScrollPage(GetRenderSize().width);
	//Range
	m_pHScroll->SetScrollRange(0, GetRenderContentSize().width);

	//VScroll
	//Position
	CRectF rcClient(GetRectInWnd());
	CRectF rcVertical;
	FLOAT lineHalfWidth = m_pProp->FocusedLine->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetIsVisible() ? (m_pHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	//rcVertical.bottom = rcClient.bottom - lineHalfWidth;

	m_pVScroll->OnRect(RectEvent(GetWndPtr(), rcVertical));

	//HScroll
	//Position
	CRectF rcHorizontal;
	rcHorizontal.left = rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom - ::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_pVScroll->GetIsVisible() ? (m_pVScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;
	m_pHScroll->OnRect(RectEvent(GetWndPtr(), rcHorizontal));
}


void CPdfView::Update() {}
