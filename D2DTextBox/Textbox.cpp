#include "text_stdafx.h"
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

#if ( _WIN32_WINNT < _WIN32_WINNT_WIN8 )
ITfThreadMgr* D2DTextbox::s_pThreadMgr = NULL;
#else
ITfThreadMgr2* D2DTextbox::s_pThreadMgr = NULL;
#endif
ITfKeystrokeMgr* D2DTextbox::s_pKeystrokeMgr = NULL;
TfClientId D2DTextbox::s_tfClientId = TF_CLIENTID_NULL;

// STATIC
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
// STATIC
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



D2DTextbox::D2DTextbox(
	CGridView* pWnd,
	CTextCell* pCell,
	std::shared_ptr<CellProperty> pProp,
	std::function<std::wstring()> getter,
	std::function<void(const std::wstring&)> setter,
	std::function<void(const std::wstring&)> changed,
	std::function<void()> final)
	:m_pWnd(pWnd), m_pCell(pCell), m_pProp(pProp), m_getter(getter), m_setter(setter), m_changed(changed), m_text(getter()), m_final(final),
	m_selStart(0), m_selEnd(getter().size())
{
	// You must create this on Heap, OnStack is NG.
	_ASSERT(_CrtIsValidHeapPointer(this));

	pCompositionRenderInfo_ = NULL;
	nCompositionRenderInfo_ = 0;
	nLineCnt_ = 0;
	row_width_ = 0;
	bRecalc_ = true;

	QueryPerformanceFrequency(&m_frequency);
	if (m_pDocumentMgr) {
		s_pThreadMgr->SetFocus(m_pDocumentMgr);
	}
	m_text.StringChanged.connect([this](const NotifyStringChangedEventArgs<wchar_t>& e)->void {
		//d2dw::CRectF rcClient(GetClientRect());
		//d2dw::CRectF rcContent(GetContentRect());

		//d2dw::CSizeF szNewContent(m_pWnd->GetDirectPtr()->CalcTextSizeWithFixedWidth(*(m_pProp->Format), e.NewString, rcContent.Width()));
		//d2dw::CRectF rcNewContent(szNewContent);
		//rcNewContent.InflateRect(*(m_pProp->Padding));
		//rcNewContent.InflateRect(m_pProp->Line->Width * 0.5f);
		//CRect rcNewClient(m_pWnd->GetDirectPtr()->Dips2Pixels(rcNewContent));

		//if (rcNewClient.Height() > rcClient.Height()) {
		//	CRect rc;
		//	::GetWindowRect(m_pWnd->m_hWnd, &rc);
		//	CPoint pt(rc.TopLeft());
		//	::ScreenToClient(::GetParent(m_pWnd->m_hWnd), &pt);
		//	::MoveWindow(m_pWnd->m_hWnd, pt.x, pt.y, rc.Width(), rcNewClient.Height(), TRUE);
		//}

		m_changed(e.NewString);


		});
}


D2DTextbox::~D2DTextbox()
{
	UninitTSF();
}

void D2DTextbox::OnCreate(const CreateEvent& e)
{
	InitTSF();
}

void D2DTextbox::OnClose(const CloseEvent& e)
{
	m_final();
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
	bool heldShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

	int nSelStart;
	int nSelEnd;
	switch (e.Char) {
	case VK_LEFT:
		if (heldShift) {
			nSelStart = GetSelectionStart();
			nSelEnd = GetSelectionEnd();
			if (nSelStart > 0) {
				if (nSelStart == nSelEnd)
					m_isSelTrail = false;

				if (m_isSelTrail)
					MoveSelection(nSelStart, nSelEnd - 1, true);
				else
					MoveSelection(nSelStart - 1, nSelEnd, false);
			}
		}
		else {
			MoveSelectionPrev();
		}
		break;

	case VK_RIGHT:
		if (heldShift) {
			nSelStart = GetSelectionStart();
			nSelEnd = GetSelectionEnd();

			if (nSelStart == nSelEnd)
				m_isSelTrail = true;

			if (m_isSelTrail)
				MoveSelection(nSelStart, nSelEnd + 1, true);
			else
				MoveSelection(nSelStart + 1, nSelEnd, false);
		}
		else {
			MoveSelectionNext();
		}
		break;

	case VK_UP:
		MoveSelectionUpDown(TRUE, heldShift);
		break;

	case VK_DOWN:
		MoveSelectionUpDown(FALSE, heldShift);
		break;

	case VK_HOME:
		MoveSelectionToLineFirstEnd(TRUE, heldShift);
		break;

	case VK_END:
		MoveSelectionToLineFirstEnd(FALSE, heldShift);
		break;

	case VK_DELETE:
		nSelStart = GetSelectionStart();
		nSelEnd = GetSelectionEnd();
		if (nSelStart == nSelEnd) {
			DeleteAtSelection(FALSE);
		}
		else {
			DeleteSelection();
		}

		break;

	case VK_BACK:
		nSelStart = GetSelectionStart();
		nSelEnd = GetSelectionEnd();
		if (nSelStart == nSelEnd) {
			DeleteAtSelection(TRUE);
		}
		else {
			DeleteSelection();
		}

		break;
	case VK_ESCAPE:
		nSelEnd = GetSelectionEnd();
		MoveSelection(nSelEnd, nSelEnd);
		break;
	case 'C':
		(heldControl && Clipboard(m_pWnd->m_hWnd, L'C') ? 1 : 0);
		break;
	case 'X':
		(heldControl && Clipboard(m_pWnd->m_hWnd, L'C') ? 1 : 0);
		if (m_selEnd > m_selStart)
		{
			m_text.notify_erase(m_selStart, m_selEnd - m_selStart);
			m_selEnd = m_selStart;
		}
		break;
	case 'V':
		(heldControl && Clipboard(m_pWnd->m_hWnd, L'V') ? 1 : 0);
		break;
	case 'Z':
		break;
	case VK_INSERT:
		(heldShift && Clipboard(m_pWnd->m_hWnd, L'V') ? 1 : 0);
		break;
	case VK_RETURN:
		InsertAtSelection(L"\r\n");
		break;
	case VK_TAB:
		InsertAtSelection(L"\t");
		break;

	default:
		break;

	}
	bRecalc_ = true;
	m_pWnd->PostUpdate(Updates::Invalidate);
}

void D2DTextbox::OnLButtonDown(const LButtonDownEvent& e)
{
	m_selDragStart = (UINT)-1;

	if (MoveSelectionAtPoint(m_pWnd->GetDirectPtr()->Pixels2Dips(e.Point))) {
		InvalidateRect();
		m_selDragStart = GetSelectionStart();
	}
	else {
		int end = m_selEnd;
		MoveSelection(end, end, true);

	}
	m_pWnd->PostUpdate(Updates::Invalidate);
}

void D2DTextbox::OnLButtonUp(const LButtonUpEvent& e)
{
	UINT nSelStart = GetSelectionStart();
	UINT nSelEnd = GetSelectionEnd();

	if (MoveSelectionAtPoint(m_pWnd->GetDirectPtr()->Pixels2Dips(e.Point))) {
		UINT nNewSelStart = GetSelectionStart();
		UINT nNewSelEnd = GetSelectionEnd();

		auto bl = true;
		if (nNewSelStart < m_selDragStart)
			bl = false;

		MoveSelection((std::min)(nSelStart, nNewSelStart), (std::max)(nSelEnd, nNewSelEnd), bl); // (nNewSelStart>_uSelDragStart));
		InvalidateRect();
	}
	m_pWnd->PostUpdate(Updates::Invalidate);
}

void D2DTextbox::OnMouseMove(const MouseMoveEvent& e)
{
	if (e.Flags & MK_LBUTTON) {
		if (MoveSelectionAtPoint(m_pWnd->GetDirectPtr()->Pixels2Dips(e.Point))) {
			UINT nNewSelStart = GetSelectionStart();
			UINT nNewSelEnd = GetSelectionEnd();

			auto bl = true;
			if (nNewSelStart < m_selDragStart)
				bl = false;

			MoveSelection((std::min)(m_selDragStart, nNewSelStart), (std::max)(m_selDragStart, nNewSelEnd), bl); // (nNewSelStart>_uSelDragStart));
			InvalidateRect();
		}
	}
	m_pWnd->PostUpdate(Updates::Invalidate);
}

void D2DTextbox::OnChar(const CharEvent& e)
{
	bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

	if (heldControl) {
		return;
	}

	// normal charcter input. not TSF.
	if (e.Char >= L' ' || (e.Char == L'\r' && !m_isSingleLine)) {
		if (e.Char < 256) {
			WCHAR wc[] = { static_cast<WCHAR>(e.Char), '\0' };
			this->InsertAtSelection(wc);
			//InvalidateRect();
		}
		else {
		}
	}
	m_pWnd->PostUpdate(Updates::Invalidate);
}

//void D2DTextbox::OnKillFocus(const KillFocusEvent& e)
//{
//	m_setter(m_text);
//	OnClose(CloseEvent(nullptr, NULL, NULL));
//}


BOOL D2DTextbox::Clipboard( HWND hwnd, TCHAR ch )
{
//	if ( !OpenClipboard( hwnd ) ) return FALSE;
	
	if ( ch == L'C' ) // copy
	{
		int SelLen = m_selEnd-m_selStart;
		if (SelLen > 0 )
		{
			std::wstring strCopy = m_text.substr(m_selStart, SelLen);

			HGLOBAL hGlobal = ::GlobalAlloc(GHND | GMEM_SHARE, (strCopy.size() + 1) * sizeof(wchar_t));
			wchar_t* strMem = (wchar_t*)::GlobalLock(hGlobal);
			::GlobalUnlock(hGlobal);

			if (strMem != NULL) {
				::wcscpy_s(strMem, strCopy.size() + 1, strCopy.c_str());
				CClipboard clipboard;
				if (clipboard.Open(m_pWnd->m_hWnd) != 0) {
					clipboard.Empty();
					clipboard.SetData(CF_UNICODETEXT, hGlobal);
					clipboard.Close();
				}
			}
		}
		else
		{
			CClipboard clipboard;
			if (clipboard.Open(m_pWnd->m_hWnd) != 0) {
				clipboard.Empty();
				clipboard.SetData(CF_UNICODETEXT, NULL);
				clipboard.Close();
			}
		}
	}
	else if ( ch == L'V' ) // paste
	{
		CClipboard clipboard;
		if (clipboard.Open(m_pWnd->m_hWnd) != 0) {
			HANDLE h = clipboard.GetData(CF_UNICODETEXT);
			if (h) {
				LPWSTR s1a = (LPWSTR)GlobalLock(h);
				auto s1b = FilterInputString(s1a, lstrlen(s1a));

				UINT s1 = m_selStart;
				UINT e1 = m_selEnd;


				//			UINT rcnt;
				m_text.notify_replace(m_selStart, m_selEnd - m_selEnd, s1b);
				m_selEnd += s1b.length();
				m_selStart = m_selEnd;

				GlobalUnlock(h);
				bRecalc_ = true;
			}
			clipboard.Close();
		}
	}	
	return TRUE;

}

std::wstring D2DTextbox::FilterInputString( LPCWSTR s, UINT len )
{
	// \n->\r or \r\n->\r
	WCHAR* cb = new WCHAR[len+1];
	WCHAR* p = cb;
	for(UINT i = 0; i < len; i++ )
	{
		if ( s[i] == '\r' && s[i+1] == '\n' )
		{				
			*p++ = '\r';
			i++;
		}
		else if ( s[i] == '\n' )
			*p++ = '\r';
		else
			*p++ = s[i];
	}
	*p = 0;

	std::wstring r = cb;
	delete [] cb;

	return r;
}

void D2DTextbox::SetText(LPCWSTR str1)
{
	m_text = str1;
	m_selStart = 0;
	m_selEnd = m_text.size();
	bRecalc_ = true;
}

d2dw::CRectF D2DTextbox::GetClientRect() const
{
	return m_pCell->GetRect();
	//CRect rcClient;
	//::GetClientRect(m_pWnd->m_hWnd, &rcClient);
	//return m_pWnd->GetDirectPtr()->Pixels2Dips(rcClient);
}

d2dw::CRectF D2DTextbox::GetContentRect() const
{
	d2dw::CRectF rcContent(GetClientRect());
	rcContent.DeflateRect(m_pProp->Line->Width*0.5f);
	rcContent.DeflateRect(*(m_pProp->Padding));
	return rcContent;
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

void D2DTextbox::MoveSelection(int nSelStart, int nSelEnd, bool bTrail)
{
	if (nSelEnd < nSelStart)
		std::swap(nSelStart, nSelEnd);

	int nTextLength = (int)m_text.size();
	if (nSelStart >= nTextLength)
		nSelStart = nTextLength;

	if (nSelEnd >= nTextLength)
		nSelEnd = nTextLength;

	m_selStart = nSelStart;
	m_selEnd = nSelEnd;

	m_isSelTrail = bTrail;


	m_pTextStore->OnSelectionChange();
}

void D2DTextbox::MoveSelectionNext()
{
	int nTextLength = (int)m_text.size();

	int zCaretPos = (m_isSelTrail ? m_selEnd : m_selStart);
	zCaretPos = (std::min)(nTextLength, zCaretPos + 1); // 1:次の文字

	m_selStart = m_selEnd = zCaretPos;
	m_pTextStore->OnSelectionChange();
}

void D2DTextbox::MoveSelectionPrev()
{
	int zCaretPos = (m_isSelTrail ? m_selEnd : m_selStart);
	zCaretPos = (std::max)(0, zCaretPos - 1);

	m_selEnd = m_selStart = zCaretPos;
	m_pTextStore->OnSelectionChange();
}

BOOL D2DTextbox::MoveSelectionAtPoint(const d2dw::CPointF& pt)
{
	BOOL bRet = FALSE;
	int nSel = (int)CharPosFromPoint(pt);
	if (nSel != -1) {
		MoveSelection(nSel, nSel, true);
		bRet = TRUE;
	}
	return bRet;
}

BOOL D2DTextbox::MoveSelectionAtNearPoint(const d2dw::CPointF& pt)
{
	BOOL bRet = FALSE;
	int nSel = (int)CharPosFromNearPoint(pt);
	if (nSel != -1) {
		MoveSelection(nSel, nSel, true);
		bRet = TRUE;
	}
	return bRet;
}

BOOL D2DTextbox::MoveSelectionUpDown(BOOL bUp, bool bShiftKey)
{
	d2dw::CRectF rc;

	if (bUp) {
		if (!RectFromCharPos(m_selStart, &rc))
			return FALSE;
	} else {
		if (!RectFromCharPos(m_selEnd, &rc)) {
			//MoveSelectionNext();
			return false;
		}

	}

	d2dw::CPointF pt;
	pt.x = rc.left;
	if (bUp) {
		pt.y = rc.top - ((rc.bottom - rc.top) / 2);
		if (pt.y < 0)
			return FALSE;
	} else {
		pt.y = rc.bottom + ((rc.bottom - rc.top) / 2);
	}

	UINT nSel = bUp ? m_selEnd : m_selStart;

	// 文字間の場合
	if (MoveSelectionAtPoint(pt)) {
		if (bShiftKey) {
			UINT nSel2 = bUp ? m_selStart : m_selEnd;
			MoveSelection(nSel, nSel2, !bUp);
		}

		return TRUE;
	}

	// 行の最後のさらに後ろ側の場合
	if (MoveSelectionAtNearPoint(pt)) {
		if (bShiftKey) {
			UINT nSel2 = bUp ? m_selStart : m_selEnd;
			MoveSelection(nSel, nSel2, !bUp);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL D2DTextbox::MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey)
{
	BOOL bRet = FALSE;
	UINT nSel, nSel2;

	if (bFirst) {
		// when pushed VK_HOME
		m_startCharPos = 0;
		nSel2 = m_selEnd;
		nSel = FineFirstEndCharPosInLine(m_selStart, TRUE);
	} else {
		// when pushed VK_END
		m_startCharPos = 0;
		nSel2 = m_selStart;
		nSel = FineFirstEndCharPosInLine(m_selEnd, FALSE);
	}

	if (nSel != (UINT)-1) {
		if (bShiftKey) {
			MoveSelection(nSel, nSel2, true);
		} else {
			MoveSelection(nSel, nSel);
		}

		bRet = TRUE;
	}


	return bRet;
}

void D2DTextbox::InvalidateRect()
{
	bRecalc_ = true;
}

void D2DTextbox::CancelEdit()
{
	SetText(m_strInit.c_str());
}

BOOL D2DTextbox::InsertAtSelection(LPCWSTR psz)
{
	bRecalc_ = true;

	LONG acpOldend = m_selEnd;

	m_text.notify_replace(m_selStart, m_selEnd - m_selStart, psz);
	m_selStart += lstrlen(psz);
	m_selEnd = m_selStart;

	m_pTextStore->OnTextChange(m_selStart, acpOldend, m_selEnd);
	m_pTextStore->OnSelectionChange();
	return TRUE;
}

BOOL D2DTextbox::DeleteAtSelection(BOOL fBack)
{
	bRecalc_ = true;

	if (!fBack && (m_selEnd < (int)m_text.size())) {
		m_text.notify_erase(m_selEnd, 1);

		m_pTextStore->OnTextChange(m_selEnd, m_selEnd + 1, m_selEnd);
	}

	if (fBack && (m_selStart > 0)) {
		m_text.notify_erase(m_selStart - 1, 1);

		m_selStart--;
		m_selEnd = m_selStart;

		m_pTextStore->OnTextChange(m_selStart, m_selStart + 1, m_selStart);
		m_pTextStore->OnSelectionChange();
	}

	return TRUE;
}

BOOL D2DTextbox::DeleteSelection()
{
	bRecalc_ = true;

	LONG acpOldEnd = m_selEnd;
	m_text.notify_erase(m_selStart, m_selEnd - m_selStart);
	m_selEnd = m_selStart;

	m_pTextStore->OnTextChange(m_selStart, acpOldEnd, m_selStart);
	m_pTextStore->OnSelectionChange();

	return TRUE;
}

void D2DTextbox::ClearText() 
{
	m_text.notify_clear();
	m_selStart = m_selEnd = 0;
}


//void D2DTextbox::OnTextChange(const std::wstring& text)
//
//{
//	d2dw::CRectF rcClient(GetClientRect());
//	d2dw::CRectF rcContent(GetContentRect());
//
//	d2dw::CSizeF szNewContent(m_pWnd->GetDirectPtr()->CalcTextSizeWithFixedWidth(*(m_pProp->Format), text, rcContent.Width()));
//	d2dw::CRectF rcNewContent(szNewContent);
//	rcNewContent.InflateRect(*(m_pProp->Padding));
//	rcNewContent.InflateRect(m_pProp->Line->Width*0.5f);
//	CRect rcNewClient(m_pWnd->GetDirectPtr()->Dips2Pixels(rcNewContent));
//
//	if (rcNewClient.Height() > rcClient.Height()) {
//		CRect rc;
//		::GetWindowRect(m_pWnd->m_hWnd, &rc);
//		CPoint pt(rc.TopLeft());
//		::ScreenToClient(::GetParent(m_pWnd->m_hWnd), &pt);
//		::MoveWindow(m_pWnd->m_hWnd, pt.x, pt.y, rc.Width(), rcNewClient.Height(), TRUE);
//	}
//
//}

void D2DTextbox::Render()
{
	if (bRecalc_) {
		Layout();
		bRecalc_ = false;
	}

	int nSelStart = (int)m_selStart - m_startCharPos;
	int nSelEnd = (int)m_selEnd - m_startCharPos;

	d2dw::CRectF rc(GetContentRect());

	m_pWnd->GetDirectPtr()->DrawTextLayout(*(m_pProp->Format), m_text, rc);

	// Render selection,caret
	d2dw::CRectF rcSelCaret(rc.left, rc.top, rc.left + 1.0f, rc.top + (float)nLineHeight_);

	CaretRect xcaret(m_isSelTrail);

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

					bool blast = m_isSelTrail;

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
				if (m_isSelTrail)
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


int D2DTextbox::CharPosFromPoint(const d2dw::CPointF& pt)
{

	for (UINT r = 0; r < nLineCnt_; r++) {
		// 行内
		for (int j = 0; j < m_lineInfos[r].nCnt; j++) {
			d2dw::CRectF& rc = m_lineInfos[r].CharInfos[j].rc;

			if (rc.PtInRect(pt) || (pt.x == 0 && rc.left == 0 && rc.right == 0 && rc.top <= pt.y && pt.y <= rc.bottom)) {
				FLOAT nWidth = m_lineInfos[r].CharInfos[j].GetWidth();
				if (pt.x > m_lineInfos[r].CharInfos[j].rc.left + (nWidth * 3 / 4)) {
					return m_lineInfos[r].nPos + j + 1 + StarCharPos_;
				}
				return m_lineInfos[r].nPos + j + StarCharPos_;
			}
		}

		// 行端
		{
			if (r == 1) {
				r = 1;
			}

			int j = m_lineInfos[r].nCnt;
			d2dw::CRectF& rc = m_lineInfos[r].CharInfos[j].rc;
			if (PtInRectRightSide(rc, pt))
				return m_lineInfos[r].nPos + j + StarCharPos_;
		}
	}

	return -1;
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

UINT D2DTextbox::FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst)
{
	if (bFirst) {
		// 先頭列へ
		for (UINT i = 0; i < nLineCnt_; i++) {
			if ((m_lineInfos[i].nPos <= (int)uCurPos) && ((int)uCurPos <= m_lineInfos[i].nPos + m_lineInfos[i].nCnt)) {
				return m_lineInfos[i].nPos;
			}
		}

	} else {
		// 行の最後尾
		for (UINT i = 0; i < nLineCnt_; i++) {
			if ((m_lineInfos[i].nPos <= (int)uCurPos) && ((int)uCurPos <= m_lineInfos[i].nPos + m_lineInfos[i].nCnt)) {
				{
					UINT idx = m_lineInfos[i].nPos + m_lineInfos[i].nCnt;
					return idx;
				}
			}
		}
	}
	return (UINT)(-1);
}


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

