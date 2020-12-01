#include "PdfView.h"
#include "Pdf.h"
#include "D2DWWindow.h"
#include "Scroll.h"
#include "Debug.h"
#include <functional>
#include "ThreadPool.h"
#include "HiLibResource.h"

CPdfView::CPdfView(CD2DWControl* pParentControl, const std::shared_ptr<PdfViewProperty>& pProp)
	:CD2DWControl(pParentControl),
    m_pProp(pProp),
	m_pMachine(std::make_unique<CPdfViewStateMachine>(this)),
	m_pVScroll(std::make_unique<CVScroll>(this, pProp->VScrollPropPtr, [this](const wchar_t* name) { })),
	m_pHScroll(std::make_unique<CHScroll>(this, pProp->HScrollPropPtr, [this](const wchar_t* name) { })),
	m_scale(1.f), m_prevScale(0.f)
{
	m_scale.Subscribe([this](const FLOAT& value)
	{
		if (m_pdf->GetDocument().second == PdfDocStatus::Available) {
			if (m_prevScale) {
				m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() * value / m_prevScale);
				m_pHScroll->SetScrollPos(m_pHScroll->GetScrollPos() * value / m_prevScale);
			}
			m_prevScale = value;
		}
	});

}

CPdfView::~CPdfView() = default;

void CPdfView::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);

	m_pdf = std::make_unique<CPdf>(GetWndPtr()->GetDirectPtr());

	GetPdfRenderer = [p = CComPtr<IPdfRendererNative>(), this]() mutable->CComPtr<IPdfRendererNative>&
	{
		if (!p) {
			::PdfCreateRenderer(GetWndPtr()->GetDirectPtr()->GetDXGIDevice(), &p);
		}
		return p;
	};
}

CRectF CPdfView::GetRenderRectInWnd()
{
    CRectF rc = GetRectInWnd();
	rc.DeflateRect(m_pProp->FocusedLine->Width * 0.5f);
	rc.DeflateRect(*(m_pProp->Padding));
	return rc;
}

CSizeF CPdfView::GetRenderSize()
{
	return GetRenderRectInWnd().Size();
}

CSizeF CPdfView::GetRenderContentSize()
{
	CSizeF sz = m_pdf->GetSourceSize();
	sz.width *= m_scale.get();
	sz.height *= m_scale.get();
	return sz;
};


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
	if(::GetAsyncKeyState(VK_CONTROL)){
		m_scale.set(std::clamp(m_scale + 0.1f * e.Delta / WHEEL_DELTA, 0.1f, 5.f));
	} else {
		m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollDelta() * e.Delta / WHEEL_DELTA);
	}
}

void CPdfView::Normal_Paint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetRectInWnd());

	//PaintContent
	if (m_pdf->GetDocument().second == PdfDocStatus::Available) {
		CRectF renderRect = GetRenderRectInWnd();
		CPointF lefttopInWnd = renderRect.LeftTop();
		CPointF lefttopInRender = CPointF(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
		UINT32 curPageNo = 0;
		FLOAT maxIntersectHeight = 0.f;
		bool curPageNoDecided = false;
		//Paint pages
		for (size_t i = 0; i < m_pdf->GetPageCount(); i++) {
			if ((lefttopInRender.y >= 0 && lefttopInRender.y <= GetRenderSize().height) ||
				(lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get() >= 0 && lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get() <= GetRenderSize().height) ||
				(lefttopInRender.y <= 0 && lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get() >= GetRenderSize().height)) {
				
				//Paint page
				auto pair = m_pdf->GetPage(i)->GetBitmap([this]() { GetWndPtr()->InvalidateRect(NULL, FALSE); });
				auto rect = CRectF(
								lefttopInWnd.x + lefttopInRender.x,
								lefttopInWnd.y + lefttopInRender.y,
								lefttopInWnd.x + lefttopInRender.x + m_pdf->GetPage(i)->GetSourceSize().width * m_scale.get(),
								lefttopInWnd.y + lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get());
				switch (pair.second) {
					case PdfBmpStatus::Available:
						GetWndPtr()->GetDirectPtr()->DrawBitmap(pair.first, rect);
						break;
					default:
						GetWndPtr()->GetDirectPtr()->DrawTextLayout(*m_pProp->Format, L"Loading Page...", rect);
						break;
				}

				//Current Page
				if (!curPageNoDecided) {
					CRectF intersectRect = renderRect.IntersectRect(rect);
					if (intersectRect == rect) {
						curPageNoDecided = true;
						curPageNo = i + 1;
					} else if (intersectRect.Height() > maxIntersectHeight) {
						maxIntersectHeight = intersectRect.Height();
						curPageNo = i + 1;
					}
				}

			}

			lefttopInRender.y += m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get();
			if (lefttopInRender.y > GetRenderSize().height) {
				break;
			}
		}

		std::wstring pageText = fmt::format(L"{} / {}", curPageNo, m_pdf->GetPageCount());
		CSizeF textSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), pageText);
		GetWndPtr()->GetDirectPtr()->DrawTextLayout(*(m_pProp->Format), pageText,
			CRectF(renderRect.right - textSize.width - m_pVScroll->GetRectInWnd().Width(),
				renderRect.top,
				renderRect.right - m_pVScroll->GetRectInWnd().Width(),
				renderRect.top + textSize.height));

	} else {
		GetWndPtr()->GetDirectPtr()->DrawTextInRect(*m_pProp->Format, L"Loading Document...", GetRenderRectInWnd());
	}

	//PaintScroll
	UpdateScroll();
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);

	//Paint Focused Line
	if (GetIsFocused() ){
		CRectF rcFocus(GetRectInWnd());
		rcFocus.DeflateRect(1.0f, 1.0f);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangle(*(m_pProp->FocusedLine), rcFocus);
	}

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CPdfView::Normal_KeyDown(const KeyDownEvent& e)
{
	bool ctrl = ::GetAsyncKeyState(VK_CONTROL);
	switch (e.Char) {
		case 'O':
			if (ctrl) {
				Open();
			}
			break;
		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}

void CPdfView::Normal_SetCursor(const SetCursorEvent& e)
{
	if (VScrl_Guard_SetCursor(e) || (HScrl_Guard_SetCursor(e))) {
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		*(e.HandledPtr) = TRUE;
	} else {
		HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
		::SetCursor(hCur);
		*(e.HandledPtr) = TRUE;
	}
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
bool CPdfView::VScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_pVScroll->GetIsVisible() && m_pVScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CPdfView::VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pVScroll->GetIsVisible() && m_pVScroll->GetThumbRect().PtInRect(e.PointInWnd);
}
void CPdfView::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pVScroll->SetScrollPos(
		m_pVScroll->GetScrollPos() +
		(e.PointInWnd.y - m_pVScroll->GetStartDrag()) *
		m_pVScroll->GetScrollDistance() /
		m_pVScroll->GetRectInWnd().Height());
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}

void CPdfView::VScrlDrag_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
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
bool CPdfView::HScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_pHScroll->GetIsVisible() && m_pHScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CPdfView::HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pHScroll->GetIsVisible() && m_pHScroll->GetThumbRect().PtInRect(e.PointInWnd);
}

void CPdfView::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pHScroll->SetScrollPos(
		m_pHScroll->GetScrollPos() +
		(e.PointInWnd.x - m_pHScroll->GetStartDrag()) *
		m_pHScroll->GetScrollDistance() /
		m_pHScroll->GetRectInWnd().Width());
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CPdfView::HScrlDrag_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

/***********/
/* Panning */
/***********/
void CPdfView::Panning_OnEntry(const LButtonBeginDragEvent& e)
{
	HScrlDrag_OnEntry(e);
	VScrlDrag_OnEntry(e);
}

void CPdfView::Panning_OnExit(const LButtonEndDragEvent& e)
{
	HScrlDrag_OnExit(e);
	VScrlDrag_OnExit(e);
}

void CPdfView::Panning_MouseMove(const MouseMoveEvent& e)
{
	m_pHScroll->SetScrollPos(
		m_pHScroll->GetScrollPos() + m_pHScroll->GetStartDrag() - e.PointInWnd.x);
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));

	m_pVScroll->SetScrollPos(
		m_pVScroll->GetScrollPos() + m_pVScroll->GetStartDrag() - e.PointInWnd.y);
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));

	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
}

void CPdfView::Panning_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
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

void CPdfView::Open()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//ofn.lpstrDefExt = L"txt";

	if (!GetOpenFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			throw std::exception(FILE_LINE_FUNC);
		}
	} else {
		::ReleaseBuffer(path);
		Open(path);
	}
}


void CPdfView::Open(const std::wstring& path)
{
	m_pdf->Load(path, [this]()->void
	{
		FLOAT scaleX = GetRenderSize().width / m_pdf->GetPage(0)->GetSourceSize().width;
		FLOAT scaleY = GetRenderSize().height / m_pdf->GetPage(0)->GetSourceSize().height;
        m_scale.set((std::min)(scaleX, scaleY));

		GetWndPtr()->InvalidateRect(NULL, FALSE);
	});
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
