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
	CGridView* pWnd,
	CTextCell* pCell,
	std::shared_ptr<CellProperty> pProp,
	std::function<std::wstring()> getter,
	std::function<void(const std::wstring&)> setter,
	std::function<void(const std::wstring&)> changed,
	std::function<void(const std::wstring&)> final)
	:m_pWnd(pWnd), m_pCell(pCell), m_pProp(pProp),
	m_getter(getter), m_setter(setter), m_changed(changed), m_final(final),
	m_text(getter()),
	m_carets(getter().size(), getter().size(), getter().size(), 0, getter().size()),
	m_recalc(true)
{
	// You must create this on Heap, OnStack is NG.
	_ASSERT(_CrtIsValidHeapPointer(this));

	pCompositionRenderInfo_ = NULL;
	nCompositionRenderInfo_ = 0;
	nLineCnt_ = 0;
	row_width_ = 0;

	QueryPerformanceFrequency(&m_frequency);
	if (m_pDocumentMgr) {
		s_pThreadMgr->SetFocus(m_pDocumentMgr);
	}
	
	m_text.StringChanged.connect(
		[this](const NotifyStringChangedEventArgs<wchar_t>& e)->void {
			m_pTextStore->OnTextChange(e.StartIndex, e.OldEndIndex, e.NewEndIndex);
			m_changed(e.NewString);
			m_recalc = true;
		}
	);
	m_carets.Changed.connect(
		[this](const NotifyChangedEventArgs<std::tuple<int, int, int, int, int>>& e)->void {
			m_pTextStore->OnSelectionChange();
		}
	);

	InitTSF();
}
/**************/
/* destructor */
/**************/
D2DTextbox::~D2DTextbox()
{
	UninitTSF();
}

void D2DTextbox::InitTSF()
{
	m_pTextStore = CComPtr<CTextStore>(new CTextStore(this));
	if (!m_pTextStore) { throw std::exception(FILE_LINE_FUNC); }
	if (FAILED(s_pThreadMgr->CreateDocumentMgr(&m_pDocumentMgr))) { throw std::exception(FILE_LINE_FUNC); }
	if (FAILED(m_pDocumentMgr->CreateContext(s_tfClientId, 0, m_pTextStore, &m_pInputContext, &ecTextStore_))) {
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


void D2DTextbox::MoveCaret(const int& newPos)
{
	m_carets.notify_set(
		std::get<caret::CurCaret>(m_carets), 
		newPos,
		newPos,
		newPos,
		newPos);

	ResetCaret();
}

void D2DTextbox::MoveCaretWithShift(const int& newPos)
{
	m_carets.notify_set(
		std::get<caret::CurCaret>(m_carets),
		newPos,
		std::get<caret::AncCaret>(m_carets),
		(std::min)(std::get<caret::AncCaret>(m_carets), newPos),
		(std::max)(std::get<caret::AncCaret>(m_carets), newPos));

	ResetCaret();
}

void D2DTextbox::MoveSelection(const int& selFirst, const int& selLast)
{
	m_carets.notify_set(
		std::get<caret::CurCaret>(m_carets),
		selLast,
		selFirst,
		selFirst,
		selLast);

	ResetCaret();
}

/**************/
/* UI Message */
/**************/
void D2DTextbox::OnClose(const CloseEvent& e)
{
	m_final(m_text);
	delete this;
}

void D2DTextbox::OnPaint(const PaintEvent& e)
{
	//PaintBackground
	e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetClientRect());
	//PaintLine
	e.WndPtr->GetDirectPtr()->DrawSolidRectangle(*(m_pProp->EditLine), GetClientRect());
	//PaintContent
	Render();
	m_pWnd->Invalidate();
}

void D2DTextbox::OnKeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (e.Char) {
	case VK_LEFT:
	{
		int newPos = std::clamp(std::get<caret::CurCaret>(m_carets) - 1, 0, (int)m_text.size());
		if (shift) {
			MoveCaretWithShift(newPos);
		} else {
			MoveCaret(newPos);
		}
		break;
	}
	case VK_RIGHT:
	{
		int newPos = std::clamp(std::get<caret::CurCaret>(m_carets) + 1, 0, (int)m_text.size());
		if (shift) {
			MoveCaretWithShift(newPos);
		} else {
			MoveCaret(newPos);
		}
		break;
	}
	case VK_UP:
	{
		auto curRect = GetCharRectFromPos(std::get<caret::CurCaret>(m_carets));
		if (!curRect) break;
		d2dw::CPointF newPt(curRect.value().left, curRect.value().top - curRect.value().Height() / 2);

		auto newPos(GetCharPosFromPoint(newPt));
		if (!newPos) break;

		if (shift) {
			MoveCaretWithShift(newPos.value());
		} else {
			MoveCaret(newPos.value());
		}
		break;
	}
	case VK_DOWN:
	{
		auto curRect = GetCharRectFromPos(std::get<caret::CurCaret>(m_carets));
		if (!curRect) break;
		d2dw::CPointF newPt(curRect.value().left, curRect.value().bottom + curRect.value().Height() / 2);

		auto newPos(GetCharPosFromPoint(newPt));
		if (!newPos) break;

		if (shift) {
			MoveCaretWithShift(newPos.value());
		} else {
			MoveCaret(newPos.value());
		}
		break;
	}
	case VK_HOME:
	{
		int newPos = std::clamp(GetFirstCharPosInLine(std::get<caret::CurCaret>(m_carets)).value(), 0, (int)m_text.size());
		if (shift) {
			MoveCaretWithShift(newPos);
		} else {
			MoveCaret(newPos);
		}
		break;
	}
	case VK_END:
	{
		int newPos = std::clamp(GetLastCharPosInLine(std::get<caret::CurCaret>(m_carets)).value(), 0, (int)m_text.size());
		if (shift) {
			MoveCaretWithShift(newPos);
		} else {
			MoveCaret(newPos);
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
			MoveCaret(std::get<caret::SelBegin>(m_carets));
		}
		break;
	}
	case VK_BACK:
	{
		if (std::get<caret::SelBegin>(m_carets) == std::get<caret::SelEnd>(m_carets)) {
			if (std::get<caret::CurCaret>(m_carets) > 0) {
				m_text.notify_erase(std::get<caret::CurCaret>(m_carets) - 1, 1);
				MoveCaret(std::get<caret::CurCaret>(m_carets) - 1);
			}

		} else {
			m_text.notify_erase(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets));
			MoveCaret(std::get<caret::SelBegin>(m_carets));
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
			MoveCaret(std::get<caret::SelBegin>(m_carets));
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
		InsertAtSelection(L"\r\n");
		break;
	case VK_TAB:
		InsertAtSelection(L"\t");
		break;

	default:
		break;

	}
}

void D2DTextbox::OnLButtonDown(const LButtonDownEvent& e)
{
	d2dw::CPointF newPt(m_pWnd->GetDirectPtr()->Pixels2Dips(e.Point));
	auto newPos(GetCharPosFromPoint(newPt));
	if (!newPos) return;

	if (GetKeyState(VK_SHIFT) & 0x8000) {
		MoveCaretWithShift(newPos.value());
	} else {
		MoveCaret(newPos.value());
	}
}

void D2DTextbox::OnMouseMove(const MouseMoveEvent& e)
{
	if (e.Flags & MK_LBUTTON) {
		d2dw::CPointF newPt(m_pWnd->GetDirectPtr()->Pixels2Dips(e.Point));
		auto newPos(GetCharPosFromPoint(newPt));
		if (!newPos) return;

		MoveCaretWithShift(newPos.value());
	}
}

void D2DTextbox::OnChar(const CharEvent& e)
{
	bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

	if (heldControl) {
		return;
	}

	// normal character input. not TSF.
	if (e.Char >= L' ' || e.Char == L'\r') {
		if (e.Char < 256) {
			WCHAR wc[] = { static_cast<WCHAR>(e.Char), '\0' };
			InsertAtSelection(wc);
		}
		else {
		}
	}
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
			//str = FilterInputString(str);
			m_text.notify_replace(std::get<caret::SelBegin>(m_carets), std::get<caret::SelEnd>(m_carets) - std::get<caret::SelBegin>(m_carets), str);
			MoveCaret(std::get<caret::CurCaret>(m_carets) + str.size());
			GlobalUnlock(hGlobal);
		}
	}
	return true;
}

d2dw::CRectF D2DTextbox::GetClientRect() const
{
	return m_pCell->GetEditRect();
}

d2dw::CRectF D2DTextbox::GetContentRect() const
{
	d2dw::CRectF rcContent(GetClientRect());
	rcContent.DeflateRect(m_pProp->Line->Width*0.5f);
	rcContent.DeflateRect(*(m_pProp->Padding));
	return rcContent;
}

void D2DTextbox::ResetCaret()
{
	QueryPerformanceCounter(&m_pregtm);
	m_bCaret = true;
}

void D2DTextbox::DrawCaret(const d2dw::CRectF& rc)
{
	QueryPerformanceCounter(&m_gtm);

	float zfps = (float)(m_gtm.QuadPart - m_pregtm.QuadPart) / (float)m_frequency.QuadPart;

	if (zfps > 0.5f) {
		m_pregtm = m_gtm;
		m_bCaret = !m_bCaret;
	} else if(m_bCaret){
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
	MoveCaret(std::get<caret::CurCaret>(m_carets) + lstrlen(psz));
	return TRUE;
}

void D2DTextbox::ClearText()
{
	m_text.notify_clear();
	MoveCaret(0);
}

void D2DTextbox::Render()
{
	if (m_recalc) {
		Layout();
		m_recalc = false;
	}

	int nSelStart = (int)std::get<caret::SelBegin>(m_carets);
	int nSelEnd = (int)std::get<caret::SelEnd>(m_carets);

	d2dw::CRectF rc(GetContentRect());

	m_pWnd->GetDirectPtr()->DrawTextLayout(*(m_pProp->Format), m_text, rc);

	// Render selection,caret
	d2dw::CRectF rcSelCaret(rc.left, rc.top, rc.left + 1.0f, rc.top + (float)nLineHeight_);

	bool isSelTrail = std::get<caret::SelEnd>(m_carets) < std::get<caret::CurCaret>(m_carets);
	CaretRect xcaret(isSelTrail);

	if (nLineCnt_) {

		for (UINT r = 0; r < nLineCnt_; r++) {

			if ((nSelEnd >= m_lineInfos[r].nPos) && (nSelStart <= m_lineInfos[r].nPos + m_lineInfos[r].nCnt)) {
				int nSelStartInLine = 0;
				int nSelEndInLine = m_lineInfos[r].nCnt;

				if (nSelStart > m_lineInfos[r].nPos) {
					nSelStartInLine = nSelStart - m_lineInfos[r].nPos;
				}

				if (nSelEnd < m_lineInfos[r].nPos + m_lineInfos[r].nCnt) {
					nSelEndInLine = nSelEnd - m_lineInfos[r].nPos;

				}

				// caret, select範囲指定の場合
				if (nSelStartInLine != nSelEndInLine && nSelEndInLine != -1) {
					for (int j = nSelStartInLine; j < nSelEndInLine; j++) {
						m_pWnd->GetDirectPtr()->FillSolidRectangle(d2dw::SolidFill(d2dw::CColorF(0, 140.f / 255, 255.f / 255, 100.f / 255)), m_lineInfos[r].CharInfos[j].rc);
					}

					bool blast = isSelTrail;

					if (blast) {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelEndInLine - 1].rc;
						rcSelCaret.left = rcSelCaret.right;
					}
					else {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left;
					}
					rcSelCaret.right++;

					xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine);


				}
				else // caret, 範囲指定されてない場合
				{
					if (nSelStartInLine == m_lineInfos[r].nCnt && !m_lineInfos[r].CharInfos.empty()) {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left + 1;

						_ASSERT(rcSelCaret.bottom >= 0);
						_ASSERT(rcSelCaret.bottom < 65000);

						xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine);


					}
					else if (nSelStartInLine > -1 && !m_lineInfos[r].CharInfos.empty()) {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left + 1;

						_ASSERT(rcSelCaret.bottom >= 0);
						xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine);

					}
				}

			}

			// キャレット表示位置が確定
			if (xcaret.IsComplete(r)) {
				if (isSelTrail)
					break;

			}

			for (UINT j = 0; j < nCompositionRenderInfo_; j++) {

				if ((pCompositionRenderInfo_[j].nEnd >= m_lineInfos[r].nPos) &&
					(pCompositionRenderInfo_[j].nStart <= m_lineInfos[r].nPos + m_lineInfos[r].nCnt)) {
					UINT nCompStartInLine = 0;
					UINT nCompEndInLine = m_lineInfos[r].nCnt;
					int  nBaseLineWidth = static_cast<int>(nLineHeight_ / 18.f) + 1;

					if (pCompositionRenderInfo_[j].nStart > m_lineInfos[r].nPos)
						nCompStartInLine = pCompositionRenderInfo_[j].nStart - m_lineInfos[r].nPos;

					if (pCompositionRenderInfo_[j].nEnd < m_lineInfos[r].nPos + m_lineInfos[r].nCnt)
						nCompEndInLine = pCompositionRenderInfo_[j].nEnd - m_lineInfos[r].nPos;

					for (UINT k = nCompStartInLine; k < nCompEndInLine; k++) {
						UINT uCurrentCompPos = m_lineInfos[r].nPos + k - pCompositionRenderInfo_[j].nStart;
						BOOL bClause = FALSE;

						if (k + 1 == nCompEndInLine) {
							bClause = TRUE;
						}

						if ((pCompositionRenderInfo_[j].da.crText.type != TF_CT_NONE) &&
							(pCompositionRenderInfo_[j].da.crBk.type != TF_CT_NONE)) {
							int a = 0;
						}

						if (pCompositionRenderInfo_[j].da.lsStyle != TF_LS_NONE) {
							// 変換途中の下線の描画
							{
								d2dw::CRectF rc = m_lineInfos[r].CharInfos[k].rc;

								d2dw::CPointF pts[2];
								pts[0].x = rc.left;
								pts[0].y = rc.bottom;
								pts[1].x = rc.right - (bClause ? nBaseLineWidth : 0);
								pts[1].y = rc.bottom;
								m_pWnd->GetDirectPtr()->DrawSolidLine(*(m_pProp->Line), pts[0], pts[1]);
							}
						}
					}
				}
			}
		}

		if (xcaret.empty()) {
			//次行の文字なし先頭列
			rcSelCaret.left = rc.left;
			rcSelCaret.right = rc.left + 1;

			rcSelCaret.OffsetRect(0, nLineHeight_ * nLineCnt_);
			xcaret.Push(rcSelCaret, nLineCnt_, 0, 0);
		}


		// 情報
		int CaretRow = xcaret.row() + 1;
		int CaretCol = xcaret.col();
		int LineNumber = nLineCnt_;


		// Caret表示
		DrawCaret(xcaret.Get());

	}
	else {
		// 文字がない場合Caret表示
		DrawCaret(rcSelCaret);
	}
}

void D2DTextbox::ClearCompositionRenderInfo()
{
	if (pCompositionRenderInfo_) {
		LocalFree(pCompositionRenderInfo_);
		pCompositionRenderInfo_ = NULL;
		nCompositionRenderInfo_ = 0;
	}
}

BOOL D2DTextbox::AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda)
{
	if (pCompositionRenderInfo_) {
		void *pvNew = LocalReAlloc(pCompositionRenderInfo_,
			(nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO),
			LMEM_MOVEABLE | LMEM_ZEROINIT);
		if (!pvNew)
			return FALSE;

		pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)pvNew;
	} else {
		pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)LocalAlloc(LPTR,
			(nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO));
		if (!pCompositionRenderInfo_)
			return FALSE;
	}
	pCompositionRenderInfo_[nCompositionRenderInfo_].nStart = nStart;
	pCompositionRenderInfo_[nCompositionRenderInfo_].nEnd = nEnd;
	pCompositionRenderInfo_[nCompositionRenderInfo_].da = *pda;
	nCompositionRenderInfo_++;

	return TRUE;
}


BOOL D2DTextbox::RectFromCharPos(UINT nPos, d2dw::CRectF *prc)
{
	d2dw::CRectF& retrc = *prc;

	retrc.SetRect(0, 0, 0, 0);

	UINT sum = 0;
	UINT current_rowno = 0;
	for (UINT r = 0; r < nLineCnt_; r++) {
		if (m_lineInfos[r].nCnt)
			sum += m_lineInfos[r].nCnt + 1; // 1 is \r
		else
			sum += 1;

		current_rowno = r;
		if (nPos < sum) {
			break;
		}
	}

	UINT pos = nPos - m_lineInfos[current_rowno].nPos;

	if (pos <= (UINT)m_lineInfos[current_rowno].nCnt && m_lineInfos[current_rowno].nCnt) {
		retrc = m_lineInfos[current_rowno].CharInfos[pos].rc;
		return TRUE;
	} else if (pos > 0) {
		retrc = m_lineInfos[current_rowno].CharInfos[pos - 1].rc;
		retrc.left = retrc.right;
		return TRUE;
	} else if (sum) {
		retrc = m_lineInfos[current_rowno].CharInfos[0].rc; // 空行で先頭の場合、
		return TRUE;
	}

	return FALSE;
}


static bool PtInRectRightSide(const d2dw::CRectF& rc, const d2dw::CPointF& pt)
{
	return (rc.top <= pt.y && pt.y <= rc.bottom && rc.right <= pt.x);
}

std::vector<d2dw::CRectF> D2DTextbox::GetCharRects()
{
	d2dw::CRectF contentRect(GetContentRect());
	std::vector<d2dw::CRectF> charRects(m_pWnd->GetDirectPtr()->CalcCharRects(*(m_pProp->Format), m_text, contentRect.Size()));
	std::for_each(charRects.begin(), charRects.end(), [offset = contentRect.LeftTop()](d2dw::CRectF& rc) {rc.OffsetRect(offset); });
	charRects.emplace_back(charRects.back().right, charRects.back().top, contentRect.right, contentRect.bottom);
	return charRects;
}

std::optional<d2dw::CRectF> D2DTextbox::GetCharRectFromPos(const int& pos)
{
	if ((pos < 0) || ((int)m_text.size() < pos)) {
		return std::nullopt;
	} else {
		return std::make_optional(GetCharRects()[pos]);
	}
}

std::optional<int> D2DTextbox::GetCharPosFromPoint(const d2dw::CPointF& pt)
{
	std::vector<d2dw::CRectF> charRects(GetCharRects());
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
	std::vector<d2dw::CRectF> charRects(GetCharRects());
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
	std::vector<d2dw::CRectF> charRects(GetCharRects());
	d2dw::CRectF curRect(charRects[pos]);
	for (auto i = pos + 1; i < (int)charRects.size(); i++) {
		if (charRects[i].top != curRect.top) {
			return std::make_optional(i);
		}
	}
	return std::make_optional((int)charRects.size());

}


int D2DTextbox::CharPosFromNearPoint(const d2dw::CPointF& pt)
{
	struct st
	{
		int i, j;
	};

	std::vector<st> ar;

	for (UINT i = 0; i < nLineCnt_; i++) {
		for (int j = 0; j < m_lineInfos[i].nCnt + 1; j++) // +1 is return key
		{
			xassert(!m_lineInfos[i].CharInfos.empty());

	
			d2dw::CRectF rc = m_lineInfos[i].CharInfos[j].rc;

			if (rc.top <= pt.y && pt.y <= rc.bottom) {
				st s;
				s.i = i;
				s.j = j;
				ar.push_back(s);
			}
		}
	}


	UINT last = ar.size();

	if (last) {
		st s = ar[last - 1];

		return m_lineInfos[s.i].nPos + s.j;

	}

	return -1;
}

//UINT D2DTextbox::FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst)
//{
//	if (bFirst) {
//		// 先頭列へ
//		for (UINT i = 0; i < nLineCnt_; i++) {
//			if ((m_lineInfos[i].nPos <= (int)uCurPos) && ((int)uCurPos <= m_lineInfos[i].nPos + m_lineInfos[i].nCnt)) {
//				return m_lineInfos[i].nPos;
//			}
//		}
//
//	} else {
//		// 行の最後尾
//		for (UINT i = 0; i < nLineCnt_; i++) {
//			if ((m_lineInfos[i].nPos <= (int)uCurPos) && ((int)uCurPos <= m_lineInfos[i].nPos + m_lineInfos[i].nCnt)) {
//				{
//					UINT idx = m_lineInfos[i].nPos + m_lineInfos[i].nCnt;
//					return idx;
//				}
//			}
//		}
//	}
//	return (UINT)(-1);
//}


BOOL D2DTextbox::Layout()
{
	d2dw::CRectF contentRect(GetContentRect());
	d2dw::CSizeF contentSize(contentRect.Size());
	nLineCnt_ = 1;

	// 行数を計算

	nLineCnt_ = 0;

	{
		BOOL bNewLine = TRUE;
		for (size_t i = 0; i < m_text.size(); i++) {
			switch (m_text[i]) {
			case 0x0d://CR
			case 0x0a://LF                
				if (bNewLine) {
					nLineCnt_++;
				}
				bNewLine = TRUE;
				break;
			default:
				if (bNewLine) {
					nLineCnt_++;
				}
				bNewLine = FALSE;
				break;
			}
		}
	}

	// 行数分のLINEINFOを作成, 文字0でもLINEINFOは１つ作成

	m_lineInfos = std::vector((std::max)((UINT)1, nLineCnt_), LINEINFO());

	// Count character of each line.　文字単位にPOS,LEN,RECTを取得

	int nCurrentLine = -1;
	m_lineInfos[0].nPos = 0;
	m_lineInfos[0].nCnt = 0;
	m_lineInfos[0].CharInfos = std::vector<CHARINFO>();

	// prgLines_ の設定
	{
		UINT crlf = 0;
		int nNewLine = 1;

		for (size_t i = 0; i < m_text.size(); i++) {
			switch (m_text[i]) {
			case 0x0d://CR
			case 0x0a://LF
				nNewLine++;

				if (nNewLine == 2) {
					nCurrentLine++;
					m_lineInfos[nCurrentLine].nPos = i;
					m_lineInfos[nCurrentLine].nCnt = 0; // CRCRの場合、nCntは0		
					m_lineInfos[nCurrentLine].CharInfos = std::vector<CHARINFO>();
					nNewLine = 1;
				}


				xassert(crlf == 0 || crlf == m_text[i]); // CRLFはだめ、CRCR.. or LFLF..はOK
				crlf = m_text[i];
				break;
			default:
				if (nNewLine) {
					nCurrentLine++;
					m_lineInfos[nCurrentLine].nPos = i;
					m_lineInfos[nCurrentLine].nCnt = 1;
					m_lineInfos[nCurrentLine].CharInfos = std::vector<CHARINFO>();

				} else {
					m_lineInfos[nCurrentLine].nCnt++;

				}
				nNewLine = 0;
				crlf = 0;
				break;
			}
		}
	}

	{

		// 文字文のRECT取得
		std::vector<d2dw::CRectF> charRects = m_pWnd->GetDirectPtr()->CalcCharRects(*(m_pProp->Format), m_text, contentSize);
		if (charRects.empty()) {
			auto pLayout = m_pWnd->GetDirectPtr()->GetTextLayout(*(m_pProp->Format), m_text, contentSize);
			float x, y;
			DWRITE_HIT_TEST_METRICS tm;
			pLayout->HitTestTextPosition(0, false, &x, &y, &tm);
			nLineHeight_ = tm.height;
		} else {
			nLineHeight_ = charRects[0].Height();
		}

		// prgLines_[lineno].prgCharInfo[col].rc の設定
		{
			UINT rcIdx = 0;
			// Get the rectangle of each characters. RECTを取得
			for (UINT r = 0; r < nLineCnt_; r++) {
				m_lineInfos[r].CharInfos = std::vector<CHARINFO>();

				UINT nCnt = m_lineInfos[r].nCnt;
				if (nCnt) {
					UINT nCntK = nCnt + 1; // 改行分

					m_lineInfos[r].CharInfos = std::vector<CHARINFO>(nCntK, CHARINFO());

					UINT col;
					for (col = 0; col < nCnt; col++) {
						d2dw::CRectF rc = charRects[rcIdx++];
						rc.OffsetRect(contentRect.LeftTop());
						m_lineInfos[r].CharInfos[col].rc = rc;
					}

					// 行の最後の処理				
					{
						d2dw::CRectF rc = m_lineInfos[r].CharInfos[col - 1].rc;
						rc.left = rc.right; //前の文字の右端を左端として、rectを作成
						m_lineInfos[r].CharInfos[col].rc = rc;//No more offset
						rcIdx++;
					}
				} else // 空行
				{
					m_lineInfos[r].CharInfos = std::vector<CHARINFO>(1, CHARINFO());
					d2dw::CRectF rc = charRects[rcIdx++];

					xassert(rc.left == rc.right);
					rc.OffsetRect(contentRect.LeftTop());
					m_lineInfos[r].CharInfos[0].rc = rc;

					xassert(m_lineInfos[r].nCnt == 0);
				}
			}
		}

		if (!m_text.empty()) {
			// 1行目の文字幅の取得、右寄せのためにrow_width_必要
			row_width_ = 0;
			for (int col = 0; col < m_lineInfos[0].nCnt; col++) {
				row_width_ += m_lineInfos[0].CharInfos[col].rc.Width();
			}
		}

		return TRUE;
	}


	return FALSE;
}
