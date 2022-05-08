#include "PdfView.h"
#include "PDFiumDoc.h"
#include "D2DWWindow.h"
#include "Scroll.h"
#include "Debug.h"
#include "MyMenu.h"
#include "ResourceIDFactory.h"
#include <functional>
#include "HiLibResource.h"
#include "PDFViewport.h"
#include "PdfViewStateMachine.h"

void CPDFCaret::Move(const int page_index, const int& char_index, const CPointF& point)
{
	auto index = std::make_tuple(page_index, char_index);
	Old = Current;
	Current = index;
	Anchor =  index;
	SelectBegin = index;
	SelectEnd = index;
	Point = std::make_tuple(page_index, point);

	StartBlink();
}
void CPDFCaret::MoveWithShift(const int page_index, const int& char_index, const CPointF& point)
{
	auto compare = [](const std::tuple<int, int>& lhs, const std::tuple<int, int>& rhs)->int
	{
		if (lhs == rhs) {
			return 0;
		} else if (std::get<0>(lhs) < std::get<0>(rhs)) {
			return -1;
		} else if (std::get<0>(lhs) > std::get<0>(rhs)) {
			return 1;
		} else {
			if (std::get<1>(lhs) < std::get<1>(rhs)) {
				return -1;
			} else if (std::get<1>(lhs) > std::get<1>(rhs)) {
				return 1;
			} else {
				return 0;
			}
		}
	};

	auto index = std::make_tuple(page_index, char_index);
	Old = Current;
	Current = std::make_tuple(page_index, char_index);
	//Anchor
	SelectBegin = compare(index, Anchor) < 0 ? index : Anchor;
	SelectEnd = compare(index, Anchor) > 0 ? index : Anchor;
	Point = std::make_tuple(page_index, point);

	StartBlink();
}
void CPDFCaret::MoveSelection(const int sel_begin_page, const int& sel_begin_char, const int sel_end_page, const int& sel_end_char)
{
	auto begin_index = std::make_tuple(sel_begin_page, sel_begin_char);
	auto end_index = std::make_tuple(sel_end_page, sel_end_char);

	Old = Current;
	Current = end_index;
	Anchor = begin_index;
	SelectBegin = begin_index;
	SelectEnd = end_index;
	//Point = std::make_tuple(page_index, point);

	StartBlink();
}

/**************************/
/* Constructor/Destructor */
/**************************/

CPdfView::CPdfView(CD2DWControl* pParentControl, const std::shared_ptr<PdfViewProperty>& pProp)
	:CD2DWControl(pParentControl),
    m_pProp(pProp),
	m_pdf(),
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

/**********/
/* Normal */
/**********/

void CPdfView::Normal_LButtonDown(const LButtonDownEvent& e)
{
	auto ptInCtrl = m_viewport.WndToCtrl(e.PointInWnd);
	auto ptInDoc = m_viewport.CtrlToDoc(ptInCtrl, m_scale);
	auto [page, ptInPage] = m_viewport.DocToPage(ptInDoc);
	auto ptInPdfiumPage = m_viewport.PageToPdfiumPage(page, ptInPage);

	CSizeF sz = m_pdf->GetPage(page)->GetSourceSize();
	UNQ_FPDF_TEXTPAGE pTextPage(m_pdf->GetPDFiumPtr()->Text_UnqLoadPage(m_pdf->GetPage(page)->GetPagePtr().get()));
	int index = m_pdf->GetPDFiumPtr()->Text_GetCharIndexAtPos(pTextPage.get(), ptInPdfiumPage.x, ptInPdfiumPage.y, sz.width, sz.height);
	if (index >= 0) {
		int rect_count = m_pdf->GetPDFiumPtr()->Text_CountRects(pTextPage.get(), index, 1);
		for (int i = 0; i < rect_count; i++) {
			double left, top, right, bottom;
			m_pdf->GetPDFiumPtr()->Text_GetRect(
				pTextPage.get(),
				i,
				&left,
				&top,
				&right,
				&bottom);
			auto rcInPdfiumPage = CRectF(
				static_cast<FLOAT>(left),
				static_cast<FLOAT>(top),
				static_cast<FLOAT>(right),
				static_cast<FLOAT>(bottom));

			auto rcInPage = m_viewport.PdfiumPageToPage(page, rcInPdfiumPage);
			auto rcInDoc = m_viewport.PageToDoc(page, rcInPage);
			auto rcInCtrl = m_viewport.DocToCtrl(rcInDoc, m_scale);
			auto rcInWnd = m_viewport.CtrlToWnd(rcInCtrl);
			m_rect = rcInWnd;
		}
	}

	//auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);
	//m_pdf->LButtonDown()
	//if (GetKeyState(VK_SHIFT) & 0x8000) {
	//	MoveCaretWithShift(index.value(), point);
	//	return;
	//} else {
	//	MoveCaret(index.value(), point);
	//}
}

void CPdfView::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	CRectF renderRect = GetRenderRectInWnd();
	CPointF lefttopInWnd = renderRect.LeftTop();
	CPointF lefttopInRender = CPointF(-m_spHScroll->GetScrollPos(), -m_spVScroll->GetScrollPos());
	for (auto i = 0; i < m_pdf->GetPageCount(); i++) {
		CSizeF sz = m_pdf->GetPage(i)->GetSourceSize();
		auto rect = CRectF(
						lefttopInWnd.x + lefttopInRender.x,
						lefttopInWnd.y + lefttopInRender.y,
						lefttopInWnd.x + lefttopInRender.x + m_pdf->GetPage(i)->GetSourceSize().width * m_scale.get(),
						lefttopInWnd.y + lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get());
		if (rect.PtInRect(e.PointInWnd)) {
			UNQ_FPDF_TEXTPAGE pTextPage(m_pdf->GetPDFiumPtr()->Text_UnqLoadPage(m_pdf->GetPage(i)->GetPagePtr().get()));
			int index = m_pdf->GetPDFiumPtr()->Text_GetCharIndexAtPos(pTextPage.get(), e.PointInWnd.x - rect.left, sz.height - (e.PointInWnd.y - rect.top), sz.width, sz.height);
			if (index >= 0) {
				int rect_count = m_pdf->GetPDFiumPtr()->Text_CountRects(pTextPage.get(), index, 1);
				for (int i = 0; i < rect_count; i++) {
					double left, top, right, bottom;
					m_pdf->GetPDFiumPtr()->Text_GetRect(
						pTextPage.get(),
						i,
						&left,
						&top,
						&right,
						&bottom);

					m_rect.SetRect(
						rect.left + static_cast<FLOAT>(left),
						rect.top + static_cast<FLOAT>(sz.height - top),
						rect.left + static_cast<FLOAT>(right),
						rect.top + static_cast<FLOAT>(sz.height - bottom));
				}
			}
		}
		lefttopInRender.y += m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get();
	}

	//auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);
	//std::vector<wchar_t> delimiters{ L' ', L'\t', L'\n' };

	//if (auto index = GetActualCharPosFromPoint(newPoint)) {
	//	if (std::find(delimiters.begin(), delimiters.end(), m_text[index.value()]) == delimiters.end()) {
	//		size_t selBegin = index.value() - 1;
	//		for (; selBegin > 0; --selBegin) {
	//			if (std::find(delimiters.begin(), delimiters.end(), m_text[selBegin]) != delimiters.end()) {
	//				selBegin++;
	//				break;
	//			}
	//		}
	//		size_t selEnd = index.value() + 1;
	//		for (; selEnd < m_text.size(); ++selEnd) {
	//			if (std::find(delimiters.begin(), delimiters.end(), m_text[selEnd]) != delimiters.end()) {
	//				break;
	//			}
	//		}
	//		MoveSelection(selBegin, selEnd);
	//	}
	//}
}

void CPdfView::Normal_Paint(const PaintEvent& e)
{
	//Clip
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetRectInWnd());

	//PaintContent
	if (m_pdf) {
		CRectF renderRect = GetRenderRectInWnd();
		CPointF lefttopInWnd = renderRect.LeftTop();
		CPointF lefttopInRender = CPointF(-m_spHScroll->GetScrollPos(), -m_spVScroll->GetScrollPos());
		UINT32 curPageNo = 0;
		FLOAT maxIntersectHeight = 0.f;
		bool curPageNoDecided = false;
		//[this]() { GetWndPtr()->InvalidateRect(NULL, FALSE); }
		//Paint Pages

		//Iterate Pages
		for (auto i = 0; i < m_pdf->GetPageCount(); i++) {
			if ((lefttopInRender.y >= 0 && lefttopInRender.y <= GetRenderSize().height) ||
				(lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get() >= 0 && lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get() <= GetRenderSize().height) ||
				(lefttopInRender.y <= 0 && lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get() >= GetRenderSize().height)) {

				auto rect = CRectF(
								lefttopInWnd.x + lefttopInRender.x,
								lefttopInWnd.y + lefttopInRender.y,
								lefttopInWnd.x + lefttopInRender.x + m_pdf->GetPage(i)->GetSourceSize().width * m_scale.get(),
								lefttopInWnd.y + lefttopInRender.y + m_pdf->GetPage(i)->GetSourceSize().height * m_scale.get());
				m_pdf->GetPage(i)->Render(BitmapRenderEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, rect, m_scale));

				m_pdf->GetPage(i)->RenderHighlite(FindRenderEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, rect, m_scale, GetFind().get()));

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
			CRectF(renderRect.right - textSize.width - m_spVScroll->GetRectInWnd().Width(),
				renderRect.top,
				renderRect.right - m_spVScroll->GetRectInWnd().Width(),
				renderRect.top + textSize.height));


	} else {
	}

	//PaintCaret
	{
		auto [page_index, char_index] = m_caret.Current;
		UNQ_FPDF_TEXTPAGE pTextPage(m_pdf->GetPDFiumPtr()->Text_UnqLoadPage(m_pdf->GetPage(page_index)->GetPagePtr().get()));
		int rect_count = m_pdf->GetPDFiumPtr()->Text_CountRects(pTextPage.get(), char_index, 1);
		if (rect_count > 0) {
			double left, top, right, bottom;
			m_pdf->GetPDFiumPtr()->Text_GetRect(
				pTextPage.get(),
				0,
				&left,
				&top,
				&right,
				&bottom);
			auto rcInPdfiumPage = CRectF(
				static_cast<FLOAT>(left),
				static_cast<FLOAT>(top),
				static_cast<FLOAT>(right),
				static_cast<FLOAT>(bottom));
			auto rcCaretInWnd = m_viewport.PdfiumPageToWnd(page_index, rcInPdfiumPage, m_scale);
			rcCaretInWnd.right = rcCaretInWnd.left + 1;
			GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
			GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_pProp->Format->Color, rcCaretInWnd);
			GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		}
	}
	//PaintSelection
	{
		auto [page_begin_index, char_begin_index] = m_caret.SelectBegin;
		auto [page_end_index, char_end_index] = m_caret.SelectEnd;

		if (page_begin_index == page_end_index) {
			UNQ_FPDF_TEXTPAGE pTextPage(m_pdf->GetPDFiumPtr()->Text_UnqLoadPage(m_pdf->GetPage(page_begin_index)->GetPagePtr().get()));
			int rect_count = m_pdf->GetPDFiumPtr()->Text_CountRects(pTextPage.get(), char_begin_index, char_end_index - char_begin_index);
			for (auto i = 0; i < rect_count; i++) {
				double left, top, right, bottom;
				m_pdf->GetPDFiumPtr()->Text_GetRect(
					pTextPage.get(),
					i,
					&left,
					&top,
					&right,
					&bottom);
				auto rcInPdfiumPage = CRectF(
					static_cast<FLOAT>(left),
					static_cast<FLOAT>(top),
					static_cast<FLOAT>(right),
					static_cast<FLOAT>(bottom));
				auto rcSelectInWnd = m_viewport.PdfiumPageToWnd(page_begin_index, rcInPdfiumPage, m_scale);
				GetWndPtr()->GetDirectPtr()->FillSolidRectangle(
						*(m_pdf->GetPropPtr()->SelectedFill), rcSelectInWnd);
			}
		}
	}


	//PaintScroll
	UpdateScroll();
	m_spVScroll->OnPaint(e);
	m_spHScroll->OnPaint(e);

	//PaintScrollHighlite;
	m_pdf->RenderHighliteLine(FindRenderLineEvent(GetWndPtr()->GetDirectPtr(), &m_viewport, m_spVScroll->GetThumbRangeRect(), m_scale, GetFind().get()));

	//GetWndPtr()->GetDirectPtr()->FillSolidRectangle(
	//		*(m_pdf->GetPropPtr()->FindHighliteFill),m_rect);

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
{}

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
	auto [page, ptInPdfiumPage] = m_viewport.WndToPdfiumPage(e.PointInWnd, m_scale);
	int index = m_pdf->GetPage(page)->GetCursorCharIndexAtPos(ptInPdfiumPage);
	if (index >= 0) {
		auto rcInWnd = m_viewport.PdfiumPageToWnd(page, m_pdf->GetPage(page)->GetCursorRect(index), m_scale);
		auto point = rcInWnd.CenterPoint();
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			m_caret.MoveWithShift(page, index, point);
			return;
		} else {
			m_caret.Move(page, index, point);
		}
	}

	//auto [page, ptInPdfiumPage] = m_viewport.WndToPdfiumPage(e.PointInWnd, m_scale);

	//CSizeF sz = m_pdf->GetPage(page)->GetSourceSize();
	//UNQ_FPDF_TEXTPAGE pTextPage(m_pdf->GetPDFiumPtr()->Text_UnqLoadPage(m_pdf->GetPage(page)->GetPagePtr().get()));
	//int index = m_pdf->GetPDFiumPtr()->Text_GetCharIndexAtPos(pTextPage.get(), ptInPdfiumPage.x, ptInPdfiumPage.y, sz.width, sz.height);
	//if (index >= 0) {
	//	int rect_count = m_pdf->GetPDFiumPtr()->Text_CountRects(pTextPage.get(), index, 1);
	//	if (rect_count > 0) {
	//		double left, top, right, bottom;
	//		m_pdf->GetPDFiumPtr()->Text_GetRect(
	//			pTextPage.get(),
	//			0,
	//			&left,
	//			&top,
	//			&right,
	//			&bottom);
	//		auto rcInPdfiumPage = CRectF(
	//			static_cast<FLOAT>(left),
	//			static_cast<FLOAT>(top),
	//			static_cast<FLOAT>(right),
	//			static_cast<FLOAT>(bottom));
	//		auto rcInWnd = m_viewport.PdfiumPageToWnd(page, rcInPdfiumPage, m_scale);
	//		auto point = rcInWnd.CenterPoint();
	//		if (GetKeyState(VK_SHIFT) & 0x8000) {
	//			m_caret.MoveWithShift(page, index, point);
	//			return;
	//		} else {
	//			m_caret.Move(page, index, point);
	//		}
	//	}
	//}
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
	auto [page, ptInPdfiumPage] = m_viewport.WndToPdfiumPage(e.PointInWnd, m_scale);
	int index = m_pdf->GetPage(page)->GetCursorCharIndexAtPos(ptInPdfiumPage);
	if (index >= 0) {
		auto rcInWnd = m_viewport.PdfiumPageToWnd(page, m_pdf->GetPage(page)->GetCursorRect(index), m_scale);
		auto point = rcInWnd.CenterPoint();
		m_caret.MoveWithShift(page, index, point);
	}
	//auto [page, ptInPdfiumPage] = m_viewport.WndToPdfiumPage(e.PointInWnd, m_scale);

	//CSizeF sz = m_pdf->GetPage(page)->GetSourceSize();
	//UNQ_FPDF_TEXTPAGE pTextPage(m_pdf->GetPDFiumPtr()->Text_UnqLoadPage(m_pdf->GetPage(page)->GetPagePtr().get()));
	//int index = m_pdf->GetPDFiumPtr()->Text_GetCharIndexAtPos(pTextPage.get(), ptInPdfiumPage.x, ptInPdfiumPage.y, sz.width, sz.height);
	//if (index >= 0) {
	//	int rect_count = m_pdf->GetPDFiumPtr()->Text_CountRects(pTextPage.get(), index, 1);
	//	if (rect_count > 0) {
	//		double left, top, right, bottom;
	//		m_pdf->GetPDFiumPtr()->Text_GetRect(
	//			pTextPage.get(),
	//			0,
	//			&left,
	//			&top,
	//			&right,
	//			&bottom);
	//		auto rcInPdfiumPage = CRectF(
	//			static_cast<FLOAT>(left),
	//			static_cast<FLOAT>(top),
	//			static_cast<FLOAT>(right),
	//			static_cast<FLOAT>(bottom));
	//		auto rcInWnd = m_viewport.PdfiumPageToWnd(page, rcInPdfiumPage, m_scale);
	//		auto point = rcInWnd.CenterPoint();
	//		m_caret.MoveWithShift(page, index, point);
	//	}
	//}
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
			path, L"", 
			GetWndPtr()->GetDirectPtr(),
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
	}
}

void CPdfView::Close()
{
	m_pdf.reset();
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
