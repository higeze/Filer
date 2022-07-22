#include "PdfView.h"
#include "PDFDoc.h"
#include "PDFPage.h"
#include "PDFCaret.h"
#include "D2DWWindow.h"
#include "Scroll.h"
#include "Debug.h"
#include "MyMenu.h"
#include "ResourceIDFactory.h"
#include <functional>
#include <ranges>
#include "HiLibResource.h"
#include "PDFViewport.h"
#include "PdfViewStateMachine.h"
#include "TextBoxDialog.h"
#include "DialogProperty.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "Button.h"
#include "strconv.h"
#include "Dispatcher.h"


/**************************/
/* Constructor/Destructor */
/**************************/

CPdfView::CPdfView(CD2DWControl* pParentControl, const std::shared_ptr<PdfViewProperty>& pProp)
	:CD2DWControl(pParentControl),
    m_pProp(pProp),
	m_pdf(),
	m_caret(this),
	m_viewport(this),
	m_pMachine(std::make_unique<CPdfViewStateMachine>(this)),
	m_spVScroll(std::make_shared<CVScroll>(this, pProp->VScrollPropPtr)),
	m_spHScroll(std::make_shared<CHScroll>(this, pProp->HScrollPropPtr)),
	m_scale(1.f), m_rotate(D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT), m_prevScale(0.f), m_initialScaleMode(InitialScaleMode::None)
{
	m_path.Subscribe([this](const NotifyStringChangedEventArgs<wchar_t>& arg)
	{
		Open(arg.NewString);
	});

	m_scale.Subscribe([this](const FLOAT& value)
	{
		if (m_prevScale) {
			m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() * value / m_prevScale);
			m_spHScroll->SetScrollPos(m_spHScroll->GetScrollPos() * value / m_prevScale);
		}
		m_prevScale = value;
	});

	m_find.Subscribe([this](const NotifyStringChangedEventArgs<wchar_t>& arg)
	{
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	});

}

CPdfView::~CPdfView() = default;

/****************/
/* EventHandler */
/****************/

void CPdfView::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcVertical, rcHorizontal] = GetRects();
	m_spVScroll->OnCreate(CreateEvt(GetWndPtr(), this, rcVertical));
	m_spHScroll->OnCreate(CreateEvt(GetWndPtr(), this, rcHorizontal));
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
	if (m_pdf) {
		CSizeF sz = m_pdf->GetSourceSize();
		sz.width *= m_scale.get();
		sz.height *= m_scale.get();
		return sz;
	} else {
		return CSizeF(0.f, 0.f);
	}
};

void CPdfView::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CPdfView::OnDestroy(const DestroyEvent& e)
{
	m_caret.StopBlink();

	CD2DWControl::OnDestroy(e);
}

void CPdfView::OnEnable(const EnableEvent& e)
{
	if (e.Enable) {
		if (!m_pdf) {
			Open(m_path.get());
		}
	} else {
		if (m_pdf) {
			Close();
		}

	}
}

void CPdfView::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	UpdateScroll();
}

void CPdfView::OnMouseWheel(const MouseWheelEvent& e)
{
	if(::GetAsyncKeyState(VK_CONTROL)){
		FLOAT factor = static_cast<FLOAT>(std::pow(1.1f, (std::abs(e.Delta) / WHEEL_DELTA)));
		FLOAT multiply = (e.Delta > 0) ? factor : 1/factor;
		m_scale.set(std::clamp(m_scale.get() * multiply, 0.1f, 8.f));
	} else {
		m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() - m_spVScroll->GetScrollDelta() * e.Delta / WHEEL_DELTA);
	}
}

void CPdfView::OnWndKillFocus(const KillFocusEvent& e)
{
	m_caret.StopBlink();
}

/**********/
/* Normal */
/**********/

void CPdfView::Normal_LButtonDown(const LButtonDownEvent& e)
{
}

void CPdfView::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
}

void CPdfView::Normal_Paint(const PaintEvent& e)
{
	if (!m_pdf) { return; }
	//Clip
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetRectInWnd());

	//PaintContent
	CRectF rcInDoc = m_viewport.CtrlToDoc(m_viewport.WndToCtrl(GetRenderRectInWnd()));

	std::vector<CRectF> intersectRectsInDoc;
	std::vector<CRectF> rectsInDoc = m_pdf->GetPageRects();

	std::transform(rectsInDoc.cbegin(), rectsInDoc.cend(), std::back_inserter(intersectRectsInDoc), [rcInDoc](const CRectF& rc) { return rc.IntersectRect(rcInDoc); });
	auto iter = std::max_element(intersectRectsInDoc.cbegin(), intersectRectsInDoc.cend(), [](const CRectF& a, const CRectF& b) { return a.Height() < b.Height(); });
	int curPageNo = std::distance(intersectRectsInDoc.cbegin(), iter) + 1;
	auto first = std::find_if(intersectRectsInDoc.cbegin(), intersectRectsInDoc.cend(), [](const CRectF& rc) { return rc.Height() > 0; });
	auto last = std::find_if(intersectRectsInDoc.crbegin(), intersectRectsInDoc.crend(), [](const CRectF& rc) { return rc.Height() > 0; });

	auto begin = std::distance(intersectRectsInDoc.cbegin(), first);
	auto end = std::min(std::distance(intersectRectsInDoc.cbegin(), last.base()), m_pdf->GetPageCount());
	bool debug = m_pMachine->IsStateNormalDebug();
	m_pdf->RenderContent(
		RenderDocContentEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, m_scale, begin, end, debug));

	//Paint Find	
	m_pdf->RenderFind(
		RenderDocFindEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, GetFind().get(), begin, end));

	//Paint Page
	std::wstring pageText = fmt::format(L"{} / {}", curPageNo, m_pdf->GetPageCount());
	CSizeF textSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), pageText);
	GetWndPtr()->GetDirectPtr()->DrawTextLayout(*(m_pProp->Format), pageText,
		CRectF(GetRenderRectInWnd().right - textSize.width - m_spVScroll->GetRectInWnd().Width(),
				GetRenderRectInWnd().top,
				GetRenderRectInWnd().right - m_spVScroll->GetRectInWnd().Width(),
				GetRenderRectInWnd().top + textSize.height));

	//PaintCaret
	if (m_caret.IsCaret()) {
		auto [page_index, char_index] = m_caret.Current;
		m_pdf
			->RenderCaret(RenderDocCaretEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, page_index, char_index));
	}
	//Paint Selected Text
	m_pdf->RenderSelectedText(RenderDocSelectedTextEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, m_caret.SelectBegin, m_caret.SelectEnd));

	//PaintScroll
	UpdateScroll();
	m_spVScroll->OnPaint(e);
	m_spHScroll->OnPaint(e);

	//PaintScrollHighlite;
	m_pdf->RenderFindLine(RenderDocFindLineEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, GetFind().get(), m_spVScroll->GetThumbRangeRect()));

	//Paint Focused Line
	if (GetIsFocused() ){
		CRectF rcFocus(GetRectInWnd());
		rcFocus.DeflateRect(1.0f, 1.0f);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(*(m_pProp->FocusedLine), rcFocus);
	}

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CPdfView::Normal_KeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (e.Char) {
		case 'O':
			if (ctrl) {
				Open();
			}
			break;
	case 'A':
	{
		if (ctrl) {
			//m_carets.set(std::get<caret::CurCaret>(m_carets.get()), m_text.size(), 0, 0, m_text.size());
			//m_caretPoint.set(GetOriginCharRects()[m_text.size()].CenterPoint());
			//StartCaretBlink();
			*e.HandledPtr = TRUE;
		}
	}
	case 'C':
	{
		if (ctrl) {
			if (m_pdf) {
				m_pdf->CopyTextToClipboard(CopyDocTextEvent(GetWndPtr(), m_caret.SelectBegin, m_caret.SelectEnd));
			}
			*e.HandledPtr = TRUE;
		}
		break;
	}

		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}

void CPdfView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	//CreateMenu
	CMenu menu(::CreatePopupMenu());
	//Add Row
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateClockwise");
	mii.dwTypeData = const_cast<LPWSTR>(L"Rotate Clockwise");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateCounterClockwise");
	mii.dwTypeData = const_cast<LPWSTR>(L"Rotate Counter Clockwise");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	::SetForegroundWindow(GetWndPtr()->m_hWnd);
	int idCmd = menu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		e.PointInScreen.x,
		e.PointInScreen.y,
		GetWndPtr()->m_hWnd);

	if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateClockwise")) {
		switch (m_rotate) {
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90;
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180;
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270;
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT;
				break;
			default:
				break;
		}
	} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateCounterClockwise")) {
		switch (m_rotate) {
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270;
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT;
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90;
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270:
				m_rotate = D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180;
				break;
			default:
				break;
		}
	}
	*e.HandledPtr = TRUE;

}

void CPdfView::Normal_KillFocus(const KillFocusEvent& e)
{
	m_caret.StopBlink();
}

/*************/
/* NormalPan */
/*************/

void CPdfView::NormalPan_SetCursor(const SetCursorEvent& e)
{
	SendPtInRectReverse(&CUIElement::OnSetCursor, e);
	if (!(*e.HandledPtr)) {
		HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
		::SetCursor(hCur);
		*(e.HandledPtr) = TRUE;
	}
}

/**************/
/* NormalText */
/**************/
void CPdfView::NormalText_LButtonDown(const LButtonDownEvent& e)
{
	auto [page, ptInPdfiumPage] = m_viewport.WndToPdfiumPage(e.PointInWnd);
	if (page >= 0) {
		int index = m_pdf->GetPage(page)->GetCursorCharIndexAtPos(ptInPdfiumPage);
		if (index >= 0) {
			auto rcInWnd = m_viewport.PdfiumPageToWnd(page, m_pdf->GetPage(page)->GetCursorRect(index));
			auto point = rcInWnd.CenterPoint();
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				m_caret.MoveWithShift(page, index, point);
				return;
			} else {
				m_caret.Move(page, index, point);
			}
		} else {
			m_caret.Clear();
		}
	}
}

void CPdfView::NormalText_SetCursor(const SetCursorEvent& e)
{
	SendPtInRectReverse(&CUIElement::OnSetCursor, e);
	if (!(*e.HandledPtr)) {
		HCURSOR hCur = ::LoadCursor(NULL, IDC_IBEAM);
		::SetCursor(hCur);
		*(e.HandledPtr) = TRUE;
	}
}


/***************/
/* VScrollDrag */
/***************/
void CPdfView::VScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spVScroll->SetState(UIElementState::Dragged);
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
void CPdfView::VScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spVScroll->SetState(UIElementState::Normal);
	m_spVScroll->SetStartDrag(0.f);
}
bool CPdfView::VScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_spVScroll->GetIsVisible() && m_spVScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CPdfView::VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spVScroll->GetIsVisible() && m_spVScroll->GetThumbRect().PtInRect(e.PointInWnd);
}
void CPdfView::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spVScroll->SetScrollPos(
		m_spVScroll->GetScrollPos() +
		(e.PointInWnd.y - m_spVScroll->GetStartDrag()) *
		m_spVScroll->GetScrollDistance() /
		m_spVScroll->GetRectInWnd().Height());
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
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
	m_spHScroll->SetState(UIElementState::Dragged);
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CPdfView::HScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spHScroll->SetState(UIElementState::Normal);
	m_spHScroll->SetStartDrag(0.f);
}
bool CPdfView::HScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_spHScroll->GetIsVisible() && m_spHScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CPdfView::HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spHScroll->GetIsVisible() && m_spHScroll->GetThumbRect().PtInRect(e.PointInWnd);
}

void CPdfView::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spHScroll->SetScrollPos(
		m_spHScroll->GetScrollPos() +
		(e.PointInWnd.x - m_spHScroll->GetStartDrag()) *
		m_spHScroll->GetScrollDistance() /
		m_spHScroll->GetRectInWnd().Width());
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
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
	m_spHScroll->SetScrollPos(
		m_spHScroll->GetScrollPos() + m_spHScroll->GetStartDrag() - e.PointInWnd.x);
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));

	m_spVScroll->SetScrollPos(
		m_spVScroll->GetScrollPos() + m_spVScroll->GetStartDrag() - e.PointInWnd.y);
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));

	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
}

void CPdfView::Panning_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

bool CPdfView::Panning_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pMachine->IsStateNormalPan();
}

/************/
/* TextDrag */
/************/

void CPdfView::TextDrag_OnEntry(const LButtonBeginDragEvent& e) {}
void CPdfView::TextDrag_OnExit(const LButtonEndDragEvent& e) {}
void CPdfView::TextDrag_MouseMove(const MouseMoveEvent& e) 
{
	auto [page, ptInPdfiumPage] = m_viewport.WndToPdfiumPage(e.PointInWnd);
	if (page >= 0) {
		int index = m_pdf->GetPage(page)->GetCursorCharIndexAtPos(ptInPdfiumPage);
		if (index >= 0) {
			auto rcInWnd = m_viewport.PdfiumPageToWnd(page, m_pdf->GetPage(page)->GetCursorRect(index));
			auto point = rcInWnd.CenterPoint();
			m_caret.MoveWithShift(page, index, point);
		}
	}
}
void CPdfView::TextDrag_SetCursor(const SetCursorEvent& e) 
{
	NormalText_SetCursor(e);
}
bool CPdfView::TextDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return true;
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
	if (::PathFileExists(path.c_str())) {

		Close();

		m_path.set(path);
		HRESULT hr = CFileIsInUseImpl::s_CreateInstance(GetWndPtr()->m_hWnd, path.c_str(), FUT_DEFAULT, OF_CAP_DEFAULT, IID_PPV_ARGS(&m_pFileIsInUse));
		GetWndPtr()->AddMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE, [this](UINT,LPARAM,WPARAM,BOOL&)->LRESULT
		{
			Close();
			return 0;
		});

		m_pdf = std::make_unique<CPDFDoc>(
			m_pProp,
			[this]()->void{
				FLOAT scaleX = GetRenderSize().width / m_pdf->GetPage(0)->GetSourceSize().width;
				FLOAT scaleY = GetRenderSize().height / m_pdf->GetPage(0)->GetSourceSize().height;
				switch (m_initialScaleMode) {
					case InitialScaleMode::MinWidthHeight:
						m_scale.set((std::min)(scaleX, scaleY));
						break;
					case InitialScaleMode::Width:
						m_scale.set(scaleX);
						break;
					case InitialScaleMode::Height:
						m_scale.set(scaleY);
						break;
					default:
						m_scale.force_notify_set(1.f);
				}

				GetWndPtr()->InvalidateRect(NULL, FALSE);
			});
		try {
			m_pdf->Open(path, L"");
		}
		catch(const CPDFException& e){
		switch (e.GetError()) {
			//case FPDF_ERR_SUCCESS:
			//  break;
			//case FPDF_ERR_UNKNOWN:
			//  break;
			//case FPDF_ERR_FILE:
			//  break;
			//case FPDF_ERR_FORMAT:
			//  break;
			case FPDF_ERR_PASSWORD:
			{
				std::shared_ptr<CTextBoxDialog> spDlg = std::make_shared<CTextBoxDialog>(GetWndPtr(), std::make_shared<DialogProperty>());
				spDlg->GetTitle().set(L"Password");
				spDlg->GetTextBlockPtr()->GetText().set(ansi_to_wide(e.what()));
				spDlg->GetOKButtonPtr()->GetContent().set(L"OK");
				spDlg->GetOKButtonPtr()->GetCommand().Subscribe([this, spDlg, path]() 
					{ 
						m_pdf->Open(path, spDlg->GetTextBoxPtr()->GetText().get()); 
						//Need to call with dispatcher, otherwise remaing message in message que is not properly handled
						GetWndPtr()->GetDispatcherPtr()->PostInvoke([spDlg]() { spDlg->OnClose(CloseEvent(spDlg->GetWndPtr(), NULL, NULL)); });
					});
				spDlg->GetCancelButtonPtr()->GetContent().set(L"Cancel");
				spDlg->GetCancelButtonPtr()->GetCommand().Subscribe([this, spDlg]() 
					{ 
						//Need to call with dispatcher, otherwise remaing message in message que is not properly handled
						GetWndPtr()->GetDispatcherPtr()->PostInvoke([spDlg]() { spDlg->OnClose(CloseEvent(spDlg->GetWndPtr(), NULL, NULL)); });
					});
				spDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), CalcCenterRectF(CSizeF(300, 200))));
				GetWndPtr()->SetFocusedControlPtr(spDlg);

				break;
			}
			//case FPDF_ERR_SECURITY:
			//  break;
			//case FPDF_ERR_PAGE:
			//  break;
			default:
			m_pdf.reset();
		}



		}
	}
}

void CPdfView::Close()
{
	m_pdf.reset();
	m_caret.Clear();
	GetWndPtr()->RemoveMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE);
	m_pFileIsInUse.Release();

	//m_path.set(L"");
	//m_scale.set(1.f);
	m_prevScale = 0.f;

	m_spVScroll->Clear();
	m_spHScroll->Clear();
}

std::tuple<CRectF, CRectF> CPdfView::GetRects() const
{
	CRectF rcClient(GetRectInWnd());
	CRectF rcVertical;
	FLOAT lineHalfWidth = m_pProp->FocusedLine->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_spHScroll->GetIsVisible() ? (m_spHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);

	CRectF rcHorizontal;
	rcHorizontal.left = rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom - ::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_spVScroll->GetIsVisible() ? (m_spVScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;

	return { rcVertical, rcHorizontal };
}

void CPdfView::UpdateScroll()
{
	//VScroll
	//Page
	m_spVScroll->SetScrollPage(GetRenderSize().height);
	//Range
	m_spVScroll->SetScrollRange(0, GetRenderContentSize().height);

	//HScroll
	//Page
	m_spHScroll->SetScrollPage(GetRenderSize().width);
	//Range
	m_spHScroll->SetScrollRange(0, GetRenderContentSize().width);

	//VScroll/HScroll Rect
	auto [rcVertical, rcHorizontal] = GetRects();
	m_spVScroll->OnRect(RectEvent(GetWndPtr(), rcVertical));
	m_spHScroll->OnRect(RectEvent(GetWndPtr(), rcHorizontal));
}


void CPdfView::Update() {}
