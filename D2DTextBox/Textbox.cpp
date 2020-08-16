﻿#include "text_stdafx.h"
#include "Textbox.h"
#include "TextboxWnd.h"
#include "TextEditSink.h"
#include "TextStoreACP.h"
#include "MyClipboard.h"
#include "CellProperty.h"
#include "Direct2DWrite.h"
#include "Debug.h"
#include "UIElement.h"
#include "GridView.h"
#include "TextCell.h"
#include "Scroll.h"
#include "ScrollProperty.h"//ScrollProperty
#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "ResourceIDFactory.h"


#define TAB_WIDTH_4CHAR 4


HRESULT InitDisplayAttrbute();
HRESULT UninitDisplayAttrbute();

/*****************/
/* static member */
/*****************/

#if ( _WIN32_WINNT < _WIN32_WINNT_WIN8 )
ITfThreadMgr* D2DTextbox::s_pThreadMgr = NULL;
#else
ITfThreadMgr2* D2DTextbox::s_pThreadMgr = NULL;
#endif
ITfKeystrokeMgr* D2DTextbox::s_pKeystrokeMgr = NULL;
TfClientId D2DTextbox::s_tfClientId = TF_CLIENTID_NULL;

/*******************/
/* static function */
/*******************/
bool D2DTextbox::AppTSFInit()
{
#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr2, (void**)&s_pThreadMgr))) goto Exit;
#else
	if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&s_pThreadMgr))) goto Exit;
#endif

	if (FAILED(s_pThreadMgr->Activate(&s_tfClientId)))  goto Exit;
	if (FAILED(s_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&s_pKeystrokeMgr)))  goto Exit;
	if (FAILED(InitDisplayAttrbute()))	goto Exit;

	return true;
Exit:

	return false;

}
void D2DTextbox::AppTSFExit()
{
	UninitDisplayAttrbute();


	if (s_pThreadMgr) {
		s_pThreadMgr->Deactivate();
		s_pThreadMgr->Release();
	}

	if (s_pKeystrokeMgr)
		s_pKeystrokeMgr->Release();
}

/***************/
/* constructor */
/***************/
D2DTextbox::D2DTextbox(
	CWnd* pWnd,
	CTextCell* pCell,
	const std::shared_ptr<TextboxProperty> pProp,
	const std::wstring& text,
	std::function<void(const std::wstring&)> changed,
	std::function<void(const std::wstring&)> final)
	:m_pWnd(pWnd), m_pCell(pCell), m_pProp(pProp),
	m_changed(changed), m_final(final),
	m_text(text),
	m_carets(0, text.size(), 0, 0, text.size()),
	m_pTextMachine(std::make_unique<CTextboxStateMachine>(this)),
	m_pVScroll(std::make_unique<d2dw::CVScroll>(pWnd, pProp->VScrollPropPtr, [this](const wchar_t* name) { UpdateScroll(); UpdateRects(); })),
	m_pHScroll(std::make_unique<d2dw::CHScroll>(pWnd, pProp->HScrollPropPtr, [this](const wchar_t* name) { UpdateScroll(); UpdateRects(); }))
{
	m_caretPoint.SetPoint(0, GetLineHeight() * 0.5f),
	
	GetOriginCharRects = [this, originCharRects = std::vector<d2dw::CRectF>()](void)mutable->std::vector<d2dw::CRectF>& {
		if (originCharRects.empty()) {
			d2dw::CRectF pageRect(GetPageRect());
			d2dw::CSizeF size(m_pProp->IsWrap ? pageRect.Width() : FLT_MAX, FLT_MAX);
			
			originCharRects = m_pWnd->GetDirectPtr()->CalcCharRects(*(m_pProp->Format), m_text, size);
			if (m_text.empty()) {
				auto size = m_pWnd->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), L"");
				originCharRects.emplace_back(
					0.f, 0.f,
					size.width, size.height);
			} else if (m_text.back() == L'\n') {
				originCharRects.emplace_back(
					0.f, originCharRects.back().bottom,
					0.f, originCharRects.back().bottom + originCharRects.back().Height());
			} else {
				originCharRects.emplace_back(
					originCharRects.back().right, originCharRects.back().top,
					originCharRects.back().right, originCharRects.back().bottom);
			}
		}
		return originCharRects;
	};

	GetOriginCursorCharRects = [this, cursorCharRects = std::vector<d2dw::CRectF>()](void)mutable->std::vector<d2dw::CRectF>& {
		if (cursorCharRects.empty()) {
			d2dw::CRectF pageRect(GetPageRect());
			cursorCharRects = GetOriginCharRects();
			//Max Right
			auto maxRight = (std::max)(pageRect.Width(), std::max_element(cursorCharRects.begin(), cursorCharRects.end(),
				[](const auto& lhs, const auto& rhs) {return lhs.right < rhs.right; })->right);

			//Case of IsWrap
			/*example-1*/
			/*abcd\n   */
			/*efghijklm*/
			/*no\n     */
			/*\n       */
			/*pqrstu\n */
			/*vwxzy    */

			/*example-2*/
			/*abcd\n   */
			/*efghijklm*/
			/*no\n     */
			/*\n       */
			/*pqrstu\n */
			/*vwxzy\n  */
			/*         */

			//Case of Not IsWrap
			/*example-1    */
			/*abcd\n       */
			/*efghijklmno\n*/
			/*\n           */
			/*pqrstu\n     */
			/*vwxzy        */

			/*example-2    */
			/*abcd\n       */
			/*efghijklmno\n*/
			/*\n           */
			/*pqrstu\n     */
			/*vwxzy\n      */
			/*             */

			for (std::size_t i = 0; i < cursorCharRects.size(); i++) {
				if (m_text[i] == L'\n' || i == m_text.size()) {
					cursorCharRects[i].right = maxRight;
				}
			}
		}
		return cursorCharRects;
	};

	GetActualCharRects = [this, actualCharRects = std::vector<d2dw::CRectF>()](void)mutable->std::vector<d2dw::CRectF>& {
		if (actualCharRects.empty())
		{
			actualCharRects = GetOriginCharRects();
			d2dw::CPointF offset(GetPageRect().LeftTop());
			offset.Offset(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
			std::for_each(actualCharRects.begin(), actualCharRects.end(), [offset](auto& rc) {rc.OffsetRect(offset); });
		}
		return actualCharRects;
	};

	GetActualSelectionCharRects = [this, selectionCharRects = std::vector<d2dw::CRectF>()](void)mutable->std::vector<d2dw::CRectF>& {
		if (selectionCharRects.empty())
		{
			selectionCharRects = GetActualCharRects();
			auto size = m_pWnd->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), L"a");
			for (size_t i = 0; i < m_text.size(); i++) {
				if (m_text[i] == L'\r' || m_text[i] == L'\n') {
					selectionCharRects[i].right += size.width;
				}
			}
		}
		return selectionCharRects;
	};

	GetOriginContentRect = [this, contentRect = d2dw::CRectF()](void)mutable->d2dw::CRectF&
	{
		auto rcPage(GetPageRect());
		auto charRects(GetOriginCharRects());
		if (!charRects.empty()) {
			contentRect = d2dw::CRectF(
				charRects.front().left,
				charRects.front().top,
				(std::max)(rcPage.Width(), std::max_element(charRects.begin(), charRects.end(), [](const auto& lhs, const auto& rhs) {return lhs.right < rhs.right; })->right),
				charRects.back().bottom);
		}
		return contentRect;
	};

	GetActualContentRect = [this, contentRect = d2dw::CRectF()](void)mutable->d2dw::CRectF&
	{
		auto rcPage(GetPageRect());
		auto charRects(GetActualCharRects());
		if (!charRects.empty()) {
			contentRect = d2dw::CRectF(
				charRects.front().left,
				charRects.front().top,
				(std::max)(rcPage.right, std::max_element(charRects.begin(), charRects.end(), [](const auto& lhs, const auto& rhs) {return lhs.right < rhs.right; })->right),
				charRects.back().bottom);
		}
		return contentRect;
	};


	// You must create this on Heap, OnStack is NG.
	_ASSERT(_CrtIsValidHeapPointer(this));

	//pCompositionRenderInfo_ = NULL;
	//nCompositionRenderInfo_ = 0;
	//nLineCnt_ = 0;
	//row_width_ = 0;

//	QueryPerformanceFrequency(&m_frequency);
	if (m_pDocumentMgr) {
		s_pThreadMgr->SetFocus(m_pDocumentMgr);
	}
	
	m_text.StringChanged.connect(
		[this](const NotifyStringChangedEventArgs<wchar_t>& e)->void {
			m_pTextStore->OnTextChange(e.StartIndex, e.OldEndIndex, e.NewEndIndex);
			m_changed(e.NewString);
			UpdateScroll();
			UpdateRects();
		}
	);
	m_carets.Changed.connect(
		[this](const NotifyChangedEventArgs<std::tuple<int, int, int, int, int>>& e)->void {
			m_pTextStore->OnSelectionChange();
			EnsureVisibleCaret();
		}
	);

	InitTSF();
}
/**************/
/* destructor */
/**************/
D2DTextbox::~D2DTextbox()
{
	m_timer.stop();
	UninitTSF();
}

void D2DTextbox::InitTSF()
{
	m_pTextStore = CComPtr<CTextStore>(new CTextStore(this));
	if (!m_pTextStore) { throw std::exception(FILE_LINE_FUNC); }
	if (FAILED(s_pThreadMgr->CreateDocumentMgr(&m_pDocumentMgr))) { throw std::exception(FILE_LINE_FUNC); }
	if (FAILED(m_pDocumentMgr->CreateContext(s_tfClientId, 0, m_pTextStore, &m_pInputContext, &m_editCookie))) {
		throw std::exception(FILE_LINE_FUNC);
	}
	if (FAILED(m_pDocumentMgr->Push(m_pInputContext))) {
		throw std::exception(FILE_LINE_FUNC);
	}

	CComPtr<ITfDocumentMgr> pDocumentMgrPrev = NULL;

#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	s_pThreadMgr->SetFocus(pDocumentMgr_);
#else
	if (FAILED(s_pThreadMgr->AssociateFocus(m_pWnd->m_hWnd, m_pDocumentMgr, &pDocumentMgrPrev))) {
		throw std::exception(FILE_LINE_FUNC);
	}
#endif

	m_pTextEditSink = CComPtr<CTextEditSink>(new CTextEditSink(this));
	if (!m_pTextEditSink) { throw std::exception(FILE_LINE_FUNC); }
	m_pTextEditSink->_Advise(m_pInputContext);
}

void D2DTextbox::UninitTSF()
{
	if (m_pTextEditSink) {
		m_pTextEditSink->_Unadvise();
	}

	if (m_pDocumentMgr) {
		m_pDocumentMgr->Pop(TF_POPF_ALL);
	}
}

void D2DTextbox::Clear()
{
	m_carets.notify_set(0, 0, 0, 0, 0);
	m_text.notify_clear();

	m_pVScroll->SetScrollPage(GetPageRect().Height());
	m_pVScroll->SetScrollPos(0.f);
	m_pVScroll->SetScrollRange(0.f, GetOriginContentRect().Height());

	m_pHScroll->SetScrollPage(GetPageRect().Width());
	m_pHScroll->SetScrollPos(0.f);
	m_pHScroll->SetScrollRange(0.f, GetOriginContentRect().Width());
}



void D2DTextbox::MoveCaret(const int& index, const d2dw::CPointF& point)
{
//	auto modPos = (newPos == m_text.size() - 1 && m_text[m_text.size() - 1] == L'\n' && std::get<caret::CurCaret>(m_carets) != newPos +1) ? newPos + 1 : newPos;
	m_carets.notify_set(
		std::get<caret::CurCaret>(m_carets), 
		index,
		index,
		index,
		index);

	m_caretPoint = point;

	ResetCaret();
}

void D2DTextbox::MoveCaretWithShift(const int& index, const d2dw::CPointF& point)
{
	m_carets.notify_set(
		std::get<caret::CurCaret>(m_carets),
		index,
		std::get<caret::AncCaret>(m_carets),
		(std::min)(std::get<caret::AncCaret>(m_carets), index),
		(std::max)(std::get<caret::AncCaret>(m_carets), index));

	m_caretPoint = point;

	ResetCaret();
}

void D2DTextbox::MoveSelection(const int& selBegin, const int& selEnd)
{
	m_carets.notify_set(
		std::get<caret::CurCaret>(m_carets),
		selEnd,
		selBegin,
		selBegin,
		selEnd);

	ResetCaret();
}

void D2DTextbox::EnsureVisibleCaret()
{
	if (m_isScrollable) {
		auto pageRect = GetPageRect();
		auto contentRect = GetOriginContentRect();
		auto charRects = GetOriginCharRects();
		if (!charRects.empty()) {
			//Range
			m_pVScroll->SetScrollRange(0, contentRect.Height());
			m_pHScroll->SetScrollRange(0, contentRect.Width());
			//Pos
			auto curRect = charRects[std::get<caret::CurCaret>(m_carets)];
			auto orgRect = charRects[0];

			if ((curRect.top - m_pVScroll->GetScrollPos() + pageRect.top) < pageRect.top) {
				m_pVScroll->SetScrollPos(curRect.top - orgRect.top);
			} else if ((curRect.bottom - m_pVScroll->GetScrollPos() +pageRect.top) > pageRect.bottom) {
				m_pVScroll->SetScrollPos(curRect.bottom - orgRect.top - pageRect.Height());
			}

			if ((curRect.left - m_pHScroll->GetScrollPos() + pageRect.left) < pageRect.left) {
				m_pHScroll->SetScrollPos(curRect.left - orgRect.left);
			} else if ((curRect.right - m_pHScroll->GetScrollPos() + pageRect.left) > pageRect.right) {
				m_pHScroll->SetScrollPos(curRect.right - orgRect.left - pageRect.Width());
			}
		}
	}
}

/*******************/
/* Windows Message */
/*******************/
void D2DTextbox::OnClose(const CloseEvent& e)
{
	if (!m_isClosing) {
		m_isClosing = true;
		m_final(m_text);
	}
}

void D2DTextbox::OnRect(const RectEvent& e)
{
	UpdateScroll();
	UpdateRects();
}

void D2DTextbox::OnMouseWheel(const MouseWheelEvent& e)
{
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollDelta() * e.Delta / WHEEL_DELTA);
}

/*****************/
/* State Machine */
/*****************/


void D2DTextbox::Normal_Paint(const PaintEvent& e)
{
	//PaintBackground
	e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetClientRect());
	//PaintLine
	if (m_hasBorder) { e.WndPtr->GetDirectPtr()->DrawSolidRectangle(*(m_pProp->EditLine), GetClientRect()); }
	//PaintContent
	Render();
	//PaintScroll
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);
}

void D2DTextbox::Normal_SetFocus(const SetFocusEvent& e)
{
	m_isFirstDrawCaret = true;
}


void D2DTextbox::Normal_KillFocus(const KillFocusEvent& e)
{
	m_timer.stop();
}

void D2DTextbox::Normal_KeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (e.Char) {
	case VK_LEFT:
	{
		auto position = std::clamp(std::get<caret::CurCaret>(m_carets) - 1, 0, (int)m_text.size());
		auto point = GetOriginCharRects()[position].CenterPoint();

		if (shift) {
			MoveCaretWithShift(position, point);
		} else {
			MoveCaret(position, point);
		}
		break;
	}
	case VK_RIGHT:
	{
		auto position = std::clamp(std::get<caret::CurCaret>(m_carets) + 1, 0, (int)m_text.size());
		auto point = GetOriginCharRects()[position].CenterPoint();

		if (shift) {
			MoveCaretWithShift(position, point);
		} else {
			MoveCaret(position, point);
		}
		break;
	}
	case VK_UP:
	{
		auto curCharRect = GetOriginCharRects()[std::get<caret::CurCaret>(m_carets)];
		auto point = d2dw::CPointF(m_caretPoint.x,
			(std::max)(GetOriginCharRects().front().CenterPoint().y, m_caretPoint.y - curCharRect.Height()));
		if (auto position = GetOriginCharPosFromPoint(point)) {
			if (shift) {
				MoveCaretWithShift(position.value(), point);
			} else {
				MoveCaret(position.value(), point);
			}
		}
		break;
	}
	case VK_DOWN:
	{
		auto curCharRect = GetOriginCharRects()[std::get<caret::CurCaret>(m_carets)];
		auto point = d2dw::CPointF(m_caretPoint.x,
			(std::min)(GetOriginCharRects().back().CenterPoint().y, m_caretPoint.y + curCharRect.Height()));
		if (auto newPos = GetOriginCharPosFromPoint(point)) {
			if (shift) {
				MoveCaretWithShift(newPos.value(), point);
			} else {
				MoveCaret(newPos.value(), point);
			}
		}
		break;
	}
	case VK_HOME:
	{
		if (auto index = GetFirstCharPosInLine(std::get<caret::CurCaret>(m_carets))){
			auto point = GetOriginCharRects()[index.value()].CenterPoint();
			if (shift) {
				MoveCaretWithShift(index.value(), point);
			} else {
				MoveCaret(index.value(), point);
			}
		}
		break;
	}
	case VK_END:
	{
		if (auto index = GetLastCharPosInLine(std::get<caret::CurCaret>(m_carets))) {
			auto point = GetOriginCharRects()[index.value()].CenterPoint();
			if (shift) {
				MoveCaretWithShift(index.value(), point);
			} else {
				MoveCaret(index.value(), point);
			}
		}
		break;
	}
	case VK_DELETE:
	{
		if (std::get<caret::SelBegin>(m_carets) == std::get<caret::SelEnd>(m_carets)) {
			if (std::get<caret::CurCaret>(m_carets) < (int)m_text.size()) {
				m_text.notify_erase(std::get<caret::CurCaret>(m_carets), 1);
			}
		} else {
			m_text.notify_erase(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets));
			auto index = std::get<caret::SelBegin>(m_carets);
			auto point = GetOriginCharRects()[index].CenterPoint();
			MoveCaret(index, point);
		}
		break;
	}
	case VK_BACK:
	{
		if (std::get<caret::SelBegin>(m_carets) == std::get<caret::SelEnd>(m_carets)) {
			if (std::get<caret::CurCaret>(m_carets) > 0) {
				m_text.notify_erase(std::get<caret::CurCaret>(m_carets) - 1, 1);
				auto index = std::get<caret::CurCaret>(m_carets) - 1;
				auto point = GetOriginCharRects()[index].CenterPoint();
				MoveCaret(index, point);
			}

		} else {
			m_text.notify_erase(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets));
			auto index = std::get<caret::SelBegin>(m_carets);
			auto point = GetOriginCharRects()[index].CenterPoint();
			MoveCaret(index, point);
		}

		break;
	}
	//case VK_ESCAPE:
	//	break;
	case 'C':
	{
		if (ctrl) {
			CopySelectionToClipboard();
		}
		break;
	}
	case 'X':
	{
		if (ctrl) {
			CopySelectionToClipboard();
			m_text.notify_erase(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets));
			auto index = std::get<caret::SelBegin>(m_carets);
			auto point = GetOriginCharRects()[index].CenterPoint();
			MoveCaret(index, point);
		}
		break;
	}
	case 'V':
	case VK_INSERT:
	{
		if (ctrl) {
			PasteFromClipboard();
		}
		break;
	}
	case VK_RETURN:
		InsertAtSelection(L"\n");
		break;
	case VK_TAB:
		InsertAtSelection(L"\t");
		break;

	default:
		break;

	}
}

void D2DTextbox::Normal_LButtonDown(const LButtonDownEvent& e)
{
	auto newPoint = m_pWnd->GetDirectPtr()->Pixels2Dips(e.PointInClient);
	auto scrollPoint = d2dw::CPointF(m_pHScroll->GetScrollPos(), m_pVScroll->GetScrollPos());
	newPoint.Offset(scrollPoint);
	newPoint.Offset(m_pProp->Padding->LeftTop());

	if (auto index = GetOriginCharPosFromPoint(newPoint)) {
		auto point = GetOriginCharRects()[index.value()].CenterPoint();
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			MoveCaretWithShift(index.value(), point);
		} else {
			MoveCaret(index.value(), point);
		}
	}
}

void D2DTextbox::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	auto newPoint = m_pWnd->GetDirectPtr()->Pixels2Dips(e.PointInClient);
	auto scrollPoint = d2dw::CPointF(m_pHScroll->GetScrollPos(), m_pVScroll->GetScrollPos());
	newPoint.Offset(scrollPoint);
	newPoint.Offset(m_pProp->Padding->LeftTop());
	std::vector<wchar_t> delimiters{ L' ', L'\t', L'\n' };
	if (auto index = GetOriginCharPosFromPoint(newPoint)) {
		if (std::find(delimiters.begin(), delimiters.end(), m_text[index.value()]) == delimiters.end()) {
			size_t selBegin = index.value() - 1;
			for (; selBegin >= 0; --selBegin) {
				if (std::find(delimiters.begin(), delimiters.end(), m_text[selBegin]) != delimiters.end()) {
					selBegin++;
					break;
				}
			}
			size_t selEnd = index.value() + 1;
			for (; selEnd < m_text.size(); ++selEnd) {
				if (std::find(delimiters.begin(), delimiters.end(), m_text[selEnd]) != delimiters.end()) {
					break;
				}
			}
			MoveSelection(selBegin, selEnd);
		}
	}

}


void D2DTextbox::Normal_MouseMove(const MouseMoveEvent& e)
{
	if (e.Flags & MK_LBUTTON) {
		d2dw::CPointF newPoint(m_pWnd->GetDirectPtr()->Pixels2Dips(e.PointInClient));
		auto scrollPoint = d2dw::CPointF(m_pHScroll->GetScrollPos(), m_pVScroll->GetScrollPos());
		newPoint.Offset(scrollPoint);
		newPoint.Offset(m_pProp->Padding->LeftTop());

		if (auto index = GetOriginCharPosFromPoint(newPoint)) {
			auto point = GetOriginCharRects()[index.value()].CenterPoint();
			MoveCaretWithShift(index.value(), point);
		}
	}
}

void D2DTextbox::Normal_Char(const CharEvent& e)
{
	bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

	if (heldControl) {
		return;
	}

	// normal character input. not TSF.
	if (e.Char >= L' ') {
		if (e.Char < 256) {
			WCHAR wc[] = { static_cast<WCHAR>(e.Char), '\0' };
			InsertAtSelection(wc);
		}
		else {
		}
	}
}

void D2DTextbox::Normal_SetCursor(const SetCursorEvent& e)
{
	CPoint pt = m_pWnd->GetCursorPosInClient();
	HCURSOR hCursor = NULL;
	if (m_pVScroll->GetVisible() && m_pVScroll->GetRect().PtInRect(m_pWnd->GetDirectPtr()->Pixels2Dips(pt)) ||
		m_pHScroll->GetVisible() && m_pHScroll->GetRect().PtInRect(m_pWnd->GetDirectPtr()->Pixels2Dips(pt))) {
		hCursor = ::LoadCursor(NULL, IDC_ARROW);
	} else {
		hCursor = ::LoadCursor(NULL, IDC_IBEAM);
	}
	if (hCursor != ::GetCursor()) {
		::SetCursor(hCursor);
	}
}

void D2DTextbox2::Normal_ContextMenu(const ContextMenuEvent& e)
{
	//CreateMenu
	CMenu menu(::CreatePopupMenu());
	//Add Row
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Execute");
	mii.dwTypeData = L"Execute";
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	::SetForegroundWindow(m_pWnd->m_hWnd);
	int idCmd = menu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		e.PointInScreen.x,
		e.PointInScreen.y,
		m_pWnd->m_hWnd);

	if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Execute")) {
		auto exe = m_text.substr(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets));
		exe = ((exe.front() == L'\"') ? L"" : L"\"") + boost::algorithm::trim_copy(exe) + ((exe.back() == L'\"') ? L"" : L"\"");
		SHELLEXECUTEINFO execInfo = {};
		execInfo.cbSize = sizeof(execInfo);
		execInfo.hwnd = m_pWnd->m_hWnd;
		execInfo.lpVerb = L"open";
		execInfo.lpFile = exe.c_str();
		execInfo.nShow = SW_SHOWDEFAULT;
		::ShellExecuteEx(&execInfo);
	}
	*e.HandledPtr = TRUE;

}



/***************/
/* VScrollDrag */
/***************/
void D2DTextbox::VScrlDrag_OnEntry(const LButtonDownEvent& e)
{
	m_pVScroll->SetState(UIElementState::Dragged);
	m_pVScroll->SetStartDrag(m_pWnd->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
void D2DTextbox::VScrlDrag_OnExit()
{
	m_pVScroll->SetState(UIElementState::Normal);
	m_pVScroll->SetStartDrag(0.f);
}
bool D2DTextbox::VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_pVScroll->GetVisible() && m_pVScroll->GetThumbRect().PtInRect(m_pWnd->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}
void D2DTextbox::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pVScroll->SetScrollPos(
		m_pVScroll->GetScrollPos() +
		(m_pWnd->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y) - m_pVScroll->GetStartDrag()) *
		m_pVScroll->GetScrollDistance() /
		m_pVScroll->GetRect().Height());
	m_pVScroll->SetStartDrag(m_pWnd->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
//bool D2DTextbox::VScrl_Guard_SetCursor(const SetCursorEvent& e)
//{
//	CPoint pt;
//	::GetCursorPos(pt);
//	return m_pVScroll->GetVisible() && m_pVScroll->GetThumbRect().PtInRect(m_pWnd->GetDirectPtr()->Pixels2Dips(pt));
//}
//void D2DTextbox::VScrl_SetCursor(const SetCursorEvent& e)
//{
//	HCURSOR hCursor = ::LoadCursor(NULL, IDC_ARROW);
//	if (hCursor != ::GetCursor()) {
//		::SetCursor(hCursor);
//	}
//}



/***************/
/* HScrollDrag */
/***************/
void D2DTextbox::HScrlDrag_OnEntry(const LButtonDownEvent& e)
{
	m_pHScroll->SetState(UIElementState::Dragged);
	m_pHScroll->SetStartDrag(m_pWnd->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void D2DTextbox::HScrlDrag_OnExit()
{
	m_pHScroll->SetState(UIElementState::Normal);
	m_pHScroll->SetStartDrag(0.f);
}

bool D2DTextbox::HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_pHScroll->GetVisible() && m_pHScroll->GetThumbRect().PtInRect(m_pWnd->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}

void D2DTextbox::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pHScroll->SetScrollPos(
		m_pHScroll->GetScrollPos() +
		(m_pWnd->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x) - m_pHScroll->GetStartDrag()) *
		m_pHScroll->GetScrollDistance() /
		m_pHScroll->GetRect().Width());
	m_pHScroll->SetStartDrag(m_pWnd->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

//bool D2DTextbox::HScrl_Guard_SetCursor(const SetCursorEvent& e)
//{
//	CPoint pt;
//	::GetCursorPos(pt);
//	return m_pHScroll->GetVisible() && m_pHScroll->GetThumbRect().PtInRect(m_pWnd->GetDirectPtr()->Pixels2Dips(pt));
//}
//void D2DTextbox::HScrl_SetCursor(const SetCursorEvent& e)
//{
//	HCURSOR hCursor = ::LoadCursor(NULL, IDC_ARROW);
//	if (hCursor != ::GetCursor()) {
//		::SetCursor(hCursor);
//	}
//}

void D2DTextbox::Error_StdException(const std::exception& e)
{
	::OutputDebugStringA(e.what());

	std::string msg = fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());

	MessageBoxA(m_pWnd->m_hWnd, msg.c_str(), "Exception in StateMachine", MB_ICONWARNING);
	Clear();
}












void D2DTextbox::UpdateScroll()
{
	//VScroll
	//Page
	m_pVScroll->SetScrollPage(GetPageRect().Height());
	//Range
	m_pVScroll->SetScrollRange(0, GetOriginContentRect().Height());
	//Enable
	m_pVScroll->SetVisible(m_pVScroll->GetScrollDistance() > m_pVScroll->GetScrollPage());

	//HScroll
	//Page
	m_pHScroll->SetScrollPage(GetPageRect().Width());
	//Range
	m_pHScroll->SetScrollRange(0, GetOriginContentRect().Width());
	//Enable
	m_pHScroll->SetVisible(m_pHScroll->GetScrollDistance() > m_pHScroll->GetScrollPage());

	//VScroll
	//Position
	d2dw::CRectF rcClient(GetClientRect());
	d2dw::CRectF rcVertical;
	FLOAT lineHalfWidth = m_pProp->Line->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetVisible() ? (m_pHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	//rcVertical.bottom = rcClient.bottom - lineHalfWidth;

	m_pVScroll->SetRect(rcVertical);

	//HScroll
	//Position
	d2dw::CRectF rcHorizontal;
	rcHorizontal.left = rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom - ::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_pVScroll->GetVisible() ? (m_pVScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;
	m_pHScroll->SetRect(rcHorizontal);
}


bool D2DTextbox::CopySelectionToClipboard()
{
	int selLen = std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets);
	if (selLen > 0) {
		std::wstring strCopy = m_text.substr(std::get<caret::SelBegin>(m_carets), selLen);

		HGLOBAL hGlobal = ::GlobalAlloc(GHND | GMEM_SHARE, (strCopy.size() + 1) * sizeof(wchar_t));
		wchar_t* strMem = (wchar_t*)::GlobalLock(hGlobal);
		::GlobalUnlock(hGlobal);

		if (strMem != NULL) {
			::wcscpy_s(strMem, strCopy.size() + 1, strCopy.c_str());
			if (CClipboard clipboard; clipboard.Open(m_pWnd->m_hWnd) != 0) {
				clipboard.Empty();
				clipboard.SetData(CF_UNICODETEXT, hGlobal);
			}
		}
	} else {
		if (CClipboard clipboard; clipboard.Open(m_pWnd->m_hWnd) != 0) {
			clipboard.Empty();
			clipboard.SetData(CF_UNICODETEXT, NULL);
			clipboard.Close();
		}
	}
	return true;
}

bool D2DTextbox::PasteFromClipboard()
{
	if (CClipboard clipboard; clipboard.Open(m_pWnd->m_hWnd) != 0) {
		HANDLE hGlobal = clipboard.GetData(CF_UNICODETEXT);
		if (hGlobal) {
			std::wstring str((LPWSTR)GlobalLock(hGlobal));
			boost::algorithm::replace_all(str, L"\r\n", L"\n");
			//str = FilterInputString(str);
			m_text.notify_replace(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets), str);
			auto index = std::get<caret::SelBegin>(m_carets) + str.size();
			auto point = GetOriginCharRects()[index].CenterPoint();

			MoveCaret(index, point);
			GlobalUnlock(hGlobal);
		}
	}
	return true;
}

d2dw::CRectF D2DTextbox::GetClientRect() const
{
	return m_pCell->GetEditRect();
}

d2dw::CRectF D2DTextbox::GetPageRect() const
{
	d2dw::CRectF rcPage(GetClientRect());
	rcPage.DeflateRect(m_pProp->Line->Width * 0.5f);
	rcPage.DeflateRect(*(m_pProp->Padding));
	return rcPage;
}


void D2DTextbox::ResetCaret()
{
	m_bCaret = true;
	m_timer.run([this]()->void
		{
			m_bCaret = !m_bCaret;
			m_pWnd->InvalidateRect(NULL, FALSE);
		}, 
		std::chrono::milliseconds(::GetCaretBlinkTime()));
	m_pWnd->InvalidateRect(NULL, FALSE);
}

void D2DTextbox::DrawCaret(const d2dw::CRectF& rc)
{
	if (m_isFirstDrawCaret) {
		m_isFirstDrawCaret = false;
		ResetCaret();
	}
	if (m_bCaret) {
		m_pWnd->GetDirectPtr()->GetHwndRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_pWnd->GetDirectPtr()->FillSolidRectangle(m_pProp->Format->Color, rc);
		m_pWnd->GetDirectPtr()->GetHwndRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
}

void D2DTextbox::CancelEdit()
{
	m_text.notify_assign(m_strInit);
	m_carets.notify_set(std::get<caret::CurCaret>(m_carets), m_text.size(), m_text.size(), m_text.size(), m_text.size());
}

BOOL D2DTextbox::InsertAtSelection(LPCWSTR psz)
{
	m_text.notify_replace(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets), psz);
	auto index = std::get<caret::SelBegin>(m_carets) + lstrlen(psz);
	auto point = GetOriginCharRects()[index].CenterPoint();

	MoveCaret(index, point);
	return TRUE;
}

void D2DTextbox::ClearText()
{
	m_text.notify_clear();
	auto index = 0;
	auto point = d2dw::CPointF(0, GetLineHeight() * 0.5f);
	MoveCaret(index, point);
}

bool D2DTextbox::GetIsVisible()const
{
	return m_pCell->GetIsVisible();
}

void D2DTextbox::Render()
{
	d2dw::CRectF pageRect(GetPageRect());

	if (!m_text.empty()) {
		std::vector<d2dw::CRectF> charRects = GetActualCharRects();
		std::vector<d2dw::CRectF> selCharRects = GetActualSelectionCharRects();

		//Draw Text
		m_pWnd->GetDirectPtr()->DrawTextLayout(*(m_pProp->Format), m_text, GetActualContentRect());

		//Draw cr, lf, tab, space
		for (size_t i = 0; i < m_text.size(); i++) {
			switch (m_text[i]) {
				case L'\r':
					break;
				case L'\n':
					m_pWnd->GetDirectPtr()->DrawLineFeed(*(m_pProp->BlankLine), selCharRects[i]);
					break;
				case L'\t':
					m_pWnd->GetDirectPtr()->DrawTab(*(m_pProp->BlankLine), selCharRects[i]);
					break;
				case L' ':
					m_pWnd->GetDirectPtr()->DrawHalfSpace(*(m_pProp->BlankLine), selCharRects[i]);
				case L'　':
				default:
					break;
			}
		}

		//Draw Selection
		for (auto n = std::get<caret::SelBegin>(m_carets); n < std::get<caret::SelEnd>(m_carets); n++) {
			m_pWnd->GetDirectPtr()->FillSolidRectangle(
				d2dw::SolidFill(d2dw::CColorF(0, 140.f / 255, 255.f / 255, 100.f / 255)),
				selCharRects[n]);
		}
		//Draw Caret
		d2dw::CRectF caretRect = charRects[std::get<caret::CurCaret>(m_carets)];
		caretRect.right = caretRect.left + 1;
		DrawCaret(caretRect);
		//Draw Composition line
		for (const auto& compositionInfo : m_compositionInfos) {
			if (compositionInfo.DisplayAttribute.lsStyle != TF_LS_NONE) {
				d2dw::CPointF ptStart, ptEnd;
				for (auto n = compositionInfo.Start; n < compositionInfo.End; n++) {
					if (n == compositionInfo.Start || (charRects[n - 1].bottom + charRects[n - 1].Height() / 2.f) < charRects[n].bottom) {
						ptStart.SetPoint(charRects[n].left, charRects[n].bottom);
					}
					if (n == (compositionInfo.End - 1) || (charRects[n].bottom + charRects[n].Height() / 2.f) < charRects[n + 1].bottom) {
						ptEnd.SetPoint(charRects[n].right, charRects[n].bottom);
						m_pWnd->GetDirectPtr()->DrawSolidTriangleWave(*(m_pProp->EditLine), ptStart, ptEnd, 4.f, 8.f);
					}
				}
			}
		}
		//for (UINT j = 0; j < nCompositionRenderInfo_; j++) {
		//	if (pCompositionRenderInfo_[j].DisplayAttribute.lsStyle != TF_LS_NONE) {
		//		d2dw::CPointF ptStart, ptEnd;
		//		for (auto n = pCompositionRenderInfo_[j].Start; n < pCompositionRenderInfo_[j].End; n++) {
		//			if (n == pCompositionRenderInfo_[j].Start || (charRects[n-1].bottom + charRects[n-1].Height() / 2.f) < charRects[n].bottom) {
		//				ptStart.SetPoint(charRects[n].left, charRects[n].bottom);
		//			}
		//			if (n == (pCompositionRenderInfo_[j].End - 1) || (charRects[n].bottom + charRects[n].Height()/2.f) < charRects[n + 1].bottom) {
		//				ptEnd.SetPoint(charRects[n].right, charRects[n].bottom);
		//				m_pWnd->GetDirectPtr()->DrawSolidTriangleWave(*(m_pProp->EditLine), ptStart, ptEnd, 4.f, 8.f);
		//			}
		//		}
		//	}
		//}
	} else {
		std::vector<d2dw::CRectF> charRects = GetActualCharRects();
		//Draw Caret
		d2dw::CRectF caretRect = charRects[std::get<caret::CurCaret>(m_carets)];
		caretRect.right = caretRect.left + 1;
		DrawCaret(caretRect);
		////Draw Caret
		//d2dw::CRectF caretRect = pageRect;
		//caretRect.right = caretRect.left + 1;
		//caretRect.bottom = pageRect.top + m_pWnd->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), L"").height * 0.5f();
		//DrawCaret(caretRect);
	}
}

FLOAT D2DTextbox::GetLineHeight()
{
	return m_pWnd->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), L"").height;
}

void D2DTextbox::ClearCompositionRenderInfo()
{
	m_compositionInfos.clear();
	//if (pCompositionRenderInfo_) {
	//	LocalFree(pCompositionRenderInfo_);
	//	pCompositionRenderInfo_ = NULL;
	//	nCompositionRenderInfo_ = 0;
	//}
}

BOOL D2DTextbox::AddCompositionRenderInfo(int start, int end, TF_DISPLAYATTRIBUTE* pDisplayAttribute)
{
	m_compositionInfos.emplace_back(start, end, *pDisplayAttribute);
	//if (pCompositionRenderInfo_) {
	//	void *pvNew = LocalReAlloc(pCompositionRenderInfo_,
	//		(nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO),
	//		LMEM_MOVEABLE | LMEM_ZEROINIT);
	//	if (!pvNew)
	//		return FALSE;

	//	pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)pvNew;
	//} else {
	//	pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)LocalAlloc(LPTR,
	//		(nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO));
	//	if (!pCompositionRenderInfo_)
	//		return FALSE;
	//}
	//pCompositionRenderInfo_[nCompositionRenderInfo_].nStart = nStart;
	//pCompositionRenderInfo_[nCompositionRenderInfo_].nEnd = nEnd;
	//pCompositionRenderInfo_[nCompositionRenderInfo_].da = *pda;
	//nCompositionRenderInfo_++;

	return TRUE;
}

std::optional<int> D2DTextbox::GetOriginCharPosFromPoint(const d2dw::CPointF& pt)
{
	if (m_text.size() == 0) {
		return std::nullopt;
	}
	std::vector<d2dw::CRectF> charRects(GetOriginCursorCharRects());

	auto iter = std::find_if(charRects.begin(), charRects.end(),
		[pt](const d2dw::CRectF& rc)->bool {
			return rc.left <= pt.x && pt.x < rc.right &&
				rc.top <= pt.y && pt.y < rc.bottom;
		});
	if (iter == charRects.end()) {
		return std::nullopt;
	} else {
		return std::make_optional(std::distance(charRects.begin(), iter));
	}
}

std::optional<int> D2DTextbox::GetFirstCharPosInLine(const int& pos)
{
	std::vector<d2dw::CRectF> charRects(GetOriginCharRects());
	d2dw::CRectF curRect(charRects[pos]);
	for (auto i = pos - 1; i > 0; i--) {
		if (charRects[i].top != curRect.top) {
			return std::make_optional(i + 1);
		}
	}
	return std::make_optional(0);
}

std::optional<int> D2DTextbox::GetLastCharPosInLine(const int& pos)
{
	std::vector<d2dw::CRectF> charRects(GetOriginCharRects());
	d2dw::CRectF curRect(charRects[pos]);
	for (auto i = pos + 1; i < (int)charRects.size(); i++) {
		if (charRects[i].top != curRect.top) {
			return std::make_optional(i);
		}
	}
	return std::make_optional((int)charRects.size());

}

void D2DTextbox::UpdateRects()
{
	GetOriginCharRects().clear();
	GetOriginCursorCharRects().clear();
	GetActualCharRects().clear();
	GetActualSelectionCharRects().clear();
}

void D2DTextbox::Update()
{
	UpdateScroll();
	UpdateRects();
	EnsureVisibleCaret();
}