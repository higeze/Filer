#include "text_stdafx.h"
#include "D2DWindow.h"
#include "Textbox.h"
#include "TextEditSink.h"
#include "TextStoreACP.h"
#include "MyClipboard.h"
#include "IBridgeTSFInterface.h"
#include "D2DContextEx.h"
#include "CellProperty.h"
#include "Direct2DWrite.h"
#include "Debug.h"


#define TAB_WIDTH_4CHAR 4

D2DTextbox::D2DTextbox(D2DWindow* pWnd, const std::shared_ptr<CellProperty>& pProp, std::function<void(const std::wstring&)> changed)
	:m_pWnd(pWnd), m_pProp(pProp), m_changed(changed)
{
	// You must create this on Heap, OnStack is NG.
	_ASSERT(_CrtIsValidHeapPointer(this));

	InitTSF();
	m_pTextEditSink->OnChanged_ = std::bind(&D2DTextbox::OnEditChanged, this);// &CTextEditorCtrl::OnChanged;

	pCompositionRenderInfo_ = NULL;
	nCompositionRenderInfo_ = 0;
	nLineCnt_ = 0;
	row_width_ = 0;
	bRecalc_ = true;
	selected_halftone_color_ = D2RGBA(0, 140, 255, 100);
	QueryPerformanceFrequency(&m_frequency);
	SetFocus();

}

D2DTextbox::~D2DTextbox()
{
	UninitTSF();
}

void D2DTextbox::CreateWindow(D2DWindow* parent, int stat, LPCWSTR name)
{
	m_pWnd = parent;
	SetText(L"");
}

void D2DTextbox::DestroyControl()
{
	SendMessage(m_pWnd->m_hWnd, WM_D2D_DESTROY_CONTROL, 0, (LPARAM)this);
}

LRESULT D2DTextbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	switch ( message )
	{
		case WM_PAINT:
		{
			//PaintBackground
			d->m_pDirect->FillSolidRectangle(*(d->m_spProp->NormalFill), GetClientRect());
			//PaintLine(e.Direct, rcClient)
			d->m_pDirect->GetHwndRenderTarget()->DrawRectangle(GetClientRect(), d->m_pDirect->GetColorBrush(d->m_spProp->EditLine->Color), d->m_spProp->Line->Width);
			//PaintContent(e.Direct, rcContent);
			EditWndProc(d, WM_PAINT, wParam, lParam);
			d->redraw_ = 1;
		}
		break;
		default :
		{				
			switch( message )
			{
				case WM_KEYDOWN:
				{			
					ret = OnKeyDown(d,message,wParam,lParam);

					if ( ret )
						return ret;

				}
				break;

			}
			int bAddTabCount = 0;
			
			if ( message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MOUSEMOVE  
				|| message == WM_RBUTTONDOWN || message == WM_RBUTTONUP || message == WM_LBUTTONDBLCLK )			
			{					
				//FPointF npt = matEx_.DPtoLP( FPointF(lParam) );
				//lParam = MAKELONG( (WORD)npt.x, (WORD)npt.y );				
			}
			else if ( message == WM_CHAR && (WCHAR)wParam == L'\r' )
			{
				// top column, tab count.
				bAddTabCount = TabCountCurrentRow();
			}

			ret = EditWndProc( d, message, wParam, lParam ); // WM_CHAR,WM_KEYDOWNの処理など

			// add tab
			for( int i = 0; i < bAddTabCount; i++ )
				OnKeyDown(d,WM_KEYDOWN,(WPARAM)L'\t',0);
		}
		break;	
	}
	return ret; 
}

int D2DTextbox::TabCountCurrentRow()
{
	int bAddTabCount = 0;

	LPCWSTR s = m_text.c_str();
	int pos = (std::max)(0, m_selEnd - 1 );

	while(pos)
	{
		if ( s[pos] == L'\n' || s[pos] == L'\r')
			break;

		if ( s[pos] == L'\t' )
			bAddTabCount++;
		else
			bAddTabCount = 0;

		pos--;
	}				
	return bAddTabCount;

}
int D2DTextbox::OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	int ret = 0;
	bool heldShift   = (GetKeyState(VK_SHIFT)   & 0x80) != 0;
    bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;
	switch( wParam )
	{
		case 'C':		
			ret =  (heldControl && Clipboard( d->m_hWnd, L'C' ) ? 1 : 0 );
		break;
		case 'X':		
			ret =  (heldControl && Clipboard( d->m_hWnd, L'C' ) ? 1 : 0 );
			if ( m_selEnd > m_selStart )
			{
				m_text.erase( m_selStart, m_selEnd-m_selStart );
				m_selEnd = m_selStart;
			}
		break;
		case 'V':
			ret =  (heldControl && Clipboard( d->m_hWnd, L'V' ) ? 1 : 0 );			 
		break;
		case 'Z':
		break;
		case VK_INSERT:				
			ret =  (heldShift && Clipboard( d->m_hWnd, L'V' ) ? 1 : 0 );
		break;
		case VK_RETURN:
		case VK_TAB:			
			InsertText(L"\t", -1, 1);
			break;

		default :
		break;

	}

	return ret;
}
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
				m_text.insert(m_selEnd, s1b);
				m_selEnd += s1b.length();
				m_selStart = m_selEnd;

				GlobalUnlock(h);


				// 上書された分を削除
				if (e1 - s1 > 0)
					m_text.erase(s1, e1 - s1);

				CalcRender(m_pWnd->cxt_);
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

void D2DTextbox::SetText(VARIANT v)
{
	if ( v.vt == VT_BSTR )
		SetText( v.bstrVal );
	else
	{
		_variant_t dst;
		if ( VariantChangeType( &dst, &v, 0, VT_BSTR ) == S_OK)
			SetText( dst.bstrVal );
	}
}
int D2DTextbox::InsertText( LPCWSTR str, int pos, int strlen)
{
	m_text.insert(pos, str, strlen);
	CalcRender(m_pWnd->cxt_);

	return 0;
}


void D2DTextbox::SetText(LPCWSTR str1)
{
	m_text = str1;
	m_selStart = 0;
	m_selEnd = m_text.size();;
	CalcRender(m_pWnd->cxt_);
}

d2dw::CRectF D2DTextbox::GetClientRect() const
{
	CRect rcClient;
	::GetClientRect(m_pWnd->m_hWnd, &rcClient);
	return m_pWnd->m_pDirect->Pixels2Dips(rcClient);
}

d2dw::CRectF D2DTextbox::GetContentRect() const
{
	d2dw::CRectF rcContent(GetClientRect());
	rcContent.DeflateRect(m_pWnd->m_spProp->Line->Width*0.5f);
	rcContent.DeflateRect(*(m_pWnd->m_spProp->Padding));
	return rcContent;
}

std::wstring D2DTextbox::GetText()
{	
	return m_text;  // null terminate	
}

static ITfKeystrokeMgr *g_pKeystrokeMgr	= NULL;
TfClientId g_TfClientId	= TF_CLIENTID_NULL;

HRESULT InitDisplayAttrbute();
HRESULT UninitDisplayAttrbute();

#if ( _WIN32_WINNT < _WIN32_WINNT_WIN8 )
ITfThreadMgr* g_pThreadMgr = NULL;
#else
ITfThreadMgr2* g_pThreadMgr = NULL;
#endif

// STATIC
bool D2DTextbox::AppTSFInit()
{
	#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,IID_ITfThreadMgr2, (void**)&g_pThreadMgr))) goto Exit;    
	#else
	if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,IID_ITfThreadMgr, (void**)&g_pThreadMgr))) goto Exit;    
	#endif
	
	if (FAILED(g_pThreadMgr->Activate(&g_TfClientId)))  goto Exit;    
	if (FAILED(g_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&g_pKeystrokeMgr)))  goto Exit;
	if ( FAILED(InitDisplayAttrbute()))	goto Exit;

	return true;
Exit:

	return false;

}
// STATIC
void D2DTextbox::AppTSFExit()
{
	UninitDisplayAttrbute();


	if ( g_pThreadMgr )
	{
		g_pThreadMgr->Deactivate();
		g_pThreadMgr->Release();
	}

	if ( g_pKeystrokeMgr )
		g_pKeystrokeMgr->Release();
}

// activeを黒色から即スタート

bool D2DTextbox::DrawCaret(D2DContext& cxt, const d2dw::CRectF& rc)
{
	QueryPerformanceCounter(&m_gtm);

	float zfps = (float)(m_gtm.QuadPart - m_pregtm.QuadPart) / (float)m_frequency.QuadPart;

	if (zfps > 0.4f) {
		m_pregtm = m_gtm;
		m_bCaret = !m_bCaret;
	} else {
		cxt.pWindow->m_pDirect->GetHwndRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		cxt.pWindow->m_pDirect->FillSolidRectangle((m_bCaret ? d2dw::SolidFill(0.f, 0.f, 0.f) : d2dw::SolidFill(1.f, 1.f, 1.f)), rc);
		cxt.pWindow->m_pDirect->GetHwndRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}

	return true;
}

void D2DTextbox::InitTSF()
{
	m_pTextStore = CComPtr<CTextStore>(new CTextStore(this));
	if (!m_pTextStore) { throw std::exception(FILE_LINE_FUNC); }
	if (FAILED(g_pThreadMgr->CreateDocumentMgr(&m_pDocumentMgr))) { throw std::exception(FILE_LINE_FUNC); }
	if (FAILED(m_pDocumentMgr->CreateContext(g_TfClientId, 0, m_pTextStore, &m_pInputContext, &ecTextStore_))) {
		throw std::exception(FILE_LINE_FUNC);
	}
	if (FAILED(m_pDocumentMgr->Push(m_pInputContext))) {
		throw std::exception(FILE_LINE_FUNC);
	}

	CComPtr<ITfDocumentMgr> pDocumentMgrPrev = NULL;

#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	g_pThreadMgr->SetFocus(pDocumentMgr_);
#else
	if (FAILED(g_pThreadMgr->AssociateFocus(m_pWnd->m_hWnd, m_pDocumentMgr, &pDocumentMgrPrev))) {
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
	zCaretPos = min(nTextLength, zCaretPos + 1); // 1:次の文字

	m_selStart = m_selEnd = zCaretPos;
	m_pTextStore->OnSelectionChange();
}

void D2DTextbox::MoveSelectionPrev()
{
	int zCaretPos = (m_isSelTrail ? m_selEnd : m_selStart);
	zCaretPos = max(0, zCaretPos - 1);

	m_selEnd = m_selStart = zCaretPos;
	m_pTextStore->OnSelectionChange();
}

BOOL D2DTextbox::MoveSelectionAtPoint(const CPoint& pt)
{
	BOOL bRet = FALSE;
	int nSel = (int)CharPosFromPoint(m_pWnd->m_pDirect->Pixels2Dips(pt));
	if (nSel != -1) {
		MoveSelection(nSel, nSel, true);
		bRet = TRUE;
	}
	return bRet;
}

BOOL D2DTextbox::MoveSelectionAtNearPoint(const CPoint& pt)
{
	BOOL bRet = FALSE;
	int nSel = (int)CharPosFromNearPoint(m_pWnd->m_pDirect->Pixels2Dips(pt));
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

	POINT pt;
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

BOOL D2DTextbox::InsertAtSelection(LPCWSTR psz)
{
	bRecalc_ = true;

	LONG lOldSelEnd = m_selEnd;
	m_text.erase(m_selStart, m_selEnd - m_selStart);

	m_text.insert(m_selStart, psz, lstrlen(psz));

	//m_selStart += nrCnt; // lstrlen(psz);
	m_selStart += lstrlen(psz);
	m_selEnd = m_selStart;


	LONG acs = m_selStart;
	LONG ecs = m_selEnd;
	m_pTextStore->OnTextChange(acs, lOldSelEnd, ecs);
	OnTextChange(m_text);
	m_changed(m_text);
	m_pTextStore->OnSelectionChange();
	return TRUE;
}

BOOL D2DTextbox::DeleteAtSelection(BOOL fBack)
{
	bRecalc_ = true;

	if (!fBack && (m_selEnd < (int)m_text.size())) {
		m_text.erase(m_selEnd, 1);


		LONG ecs = m_selEnd;

		m_pTextStore->OnTextChange(ecs, ecs + 1, ecs);
		OnTextChange(m_text);
		m_changed(m_text);

	}

	if (fBack && (m_selStart > 0)) {
		m_text.erase(m_selStart - 1, 1);

		m_selStart--;
		m_selEnd = m_selStart;

		LONG acs = m_selStart;
		m_pTextStore->OnTextChange(acs, acs + 1, acs);
		OnTextChange(m_text);
		m_changed(m_text);
		m_pTextStore->OnSelectionChange();
	}

	return TRUE;
}

BOOL D2DTextbox::DeleteSelection()
{
	bRecalc_ = true;

	ULONG nSelOldEnd = m_selEnd;
	m_text.erase(m_selStart, m_selEnd - m_selStart);

	m_selEnd = m_selStart;

	LONG acs = m_selStart;

	m_pTextStore->OnTextChange(acs, nSelOldEnd, acs);
	OnTextChange(m_text);
	m_changed(m_text);
	m_pTextStore->OnSelectionChange();

	return TRUE;
}

void D2DTextbox::OnTextChange(const std::wstring& text)
{
	d2dw::CRectF rcClient(GetClientRect());
	d2dw::CRectF rcContent(GetContentRect());

	d2dw::CSizeF szNewContent(m_pWnd->m_pDirect->CalcTextSizeWithFixedWidth(*(m_pProp->Format), text, rcContent.Width()));
	d2dw::CRectF rcNewContent(szNewContent);
	rcNewContent.InflateRect(*(m_pProp->Padding));
	rcNewContent.InflateRect(m_pProp->Line->Width*0.5f);
	CRect rcNewClient(m_pWnd->m_pDirect->Dips2Pixels(rcNewContent));

	if (rcNewClient.Height() > rcClient.Height()) {
		CRect rc;
		::GetWindowRect(m_pWnd->m_hWnd, &rc);
		CPoint pt(rc.TopLeft());
		::ScreenToClient(::GetParent(m_pWnd->m_hWnd), &pt);
		::MoveWindow(m_pWnd->m_hWnd, pt.x, pt.y, rc.Width(), rcNewClient.Height(), TRUE);
	}

}

int D2DTextbox::CurrentCaretPos()
{
	return (m_isSelTrail ? m_selEnd : m_selStart);
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void D2DTextbox::Render(D2DContext& cxt)
{
	int zCaretPos = CurrentCaretPos(); //(ct_->bSelTrail_ ? m_selEnd : m_selStart );

	if (bRecalc_) {
		Layout(cxt, m_text.c_str(), m_text.size(), GetContentRect().Size(), zCaretPos, m_startCharPos);
		bRecalc_ = false;
	}

	int selstart = (int)m_selStart - m_startCharPos;
	int selend = (int)m_selEnd - m_startCharPos;

	Render(cxt, GetContentRect(), m_text.c_str(), m_text.size(), selstart, selend, m_isSelTrail, pCompositionRenderInfo_, nCompositionRenderInfo_);
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void D2DTextbox::CalcRender(D2DContext& cxt)
{
	int x = 0;

	//::OutputDebugStringA((boost::format("TextBuff:%1%, TextLen:%2%\r\n") % ct_->GetTextBuffer() % ct_->GetTextLength()).str().c_str());
	d2dw::CSizeF size(GetContentRect().Size());
	Layout(cxt, m_text.c_str(), m_text.size(), size, 0, x);
	bRecalc_ = false;
}

void D2DTextbox::SetFocus()
{
	if (m_pDocumentMgr) {
		g_pThreadMgr->SetFocus(m_pDocumentMgr);
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

void D2DTextbox::OnEditChanged() {}

LRESULT D2DTextbox::EditWndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	switch (message) {
	case WM_PAINT:
		OnPaint(d->cxt_);
		break;
		/*case WM_SETFOCUS:
			focusは別系統で
			this->OnSetFocus(wParam, lParam);
		break;*/
	case WM_KEYDOWN:
		if (this->OnKeyDown(wParam, lParam))
			ret = 1;
		break;
	case WM_LBUTTONDOWN:
		this->OnLButtonDown(wParam, lParam);
		break;
	case WM_LBUTTONUP:
		this->OnLButtonUp(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		this->OnMouseMove(wParam, lParam);
		break;
	case WM_CHAR:
		// wParam is a character of the result string. 

		bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;
		WCHAR wch = (WCHAR)wParam;

		if (heldControl)
			return 0;

		// normal charcter input. not TSF.
		if (wch >= L' ' || (wch == L'\r' && !m_isSingleLine)) {
			if (wch < 256) {
				WCHAR wc[] = { wch, '\0' };
				this->InsertAtSelection(wc);
				InvalidateRect();
			} else {
				// outof input
//					TRACE( L"out of char %c\n", wch );
			}
		}
		break;
	}
	return ret;
}

void D2DTextbox::OnPaint(D2DContext& hdc)
{
	Render(hdc);
}

BOOL D2DTextbox::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	BOOL ret = true;

	bool heldShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;

	int nSelStart;
	int nSelEnd;
	switch (0xff & wParam) {
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
		} else {
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
		} else {
			MoveSelectionNext();
		}
		break;

	case VK_UP:
		ret = MoveSelectionUpDown(TRUE, heldShift);
		break;

	case VK_DOWN:
		ret = MoveSelectionUpDown(FALSE, heldShift);
		break;

	case VK_HOME:
		ret = MoveSelectionToLineFirstEnd(TRUE, heldShift);
		break;

	case VK_END:
		ret = MoveSelectionToLineFirstEnd(FALSE, heldShift);
		break;

	case VK_DELETE:
		nSelStart = GetSelectionStart();
		nSelEnd = GetSelectionEnd();
		if (nSelStart == nSelEnd) {
			DeleteAtSelection(FALSE);
		} else {
			DeleteSelection();
		}

		break;

	case VK_BACK:
		nSelStart = GetSelectionStart();
		nSelEnd = GetSelectionEnd();
		if (nSelStart == nSelEnd) {
			DeleteAtSelection(TRUE);
		} else {
			DeleteSelection();
		}

		break;
	case VK_ESCAPE:
		nSelEnd = GetSelectionEnd();
		MoveSelection(nSelEnd, nSelEnd);
		break;
	}

	bRecalc_ = true;

	return ret;
}

void D2DTextbox::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
	//SetFocus();
}

void D2DTextbox::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	CPoint pt((short)LOWORD(lParam), (short)(HIWORD(lParam)));
	m_selDragStart = (UINT)-1;

	if (MoveSelectionAtPoint(pt)) {
		InvalidateRect();
		m_selDragStart = GetSelectionStart();
	} else {
		int end = m_selEnd;
		MoveSelection(end, end, true);

	}
}

void D2DTextbox::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	UINT nSelStart = GetSelectionStart();
	UINT nSelEnd = GetSelectionEnd();
	CPoint pt((short)LOWORD(lParam), (short)HIWORD(lParam));

	if (MoveSelectionAtPoint(pt)) {
		UINT nNewSelStart = GetSelectionStart();
		UINT nNewSelEnd = GetSelectionEnd();

		auto bl = true;
		if (nNewSelStart < m_selDragStart)
			bl = false;

		MoveSelection(min(nSelStart, nNewSelStart), max(nSelEnd, nNewSelEnd), bl); // (nNewSelStart>_uSelDragStart));
		InvalidateRect();
	}
}

void D2DTextbox::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	if (wParam & MK_LBUTTON) {
		CPoint pt((std::max)((short)0, (short)LOWORD(lParam)), (std::max)((short)0, (short)HIWORD(lParam)));

		if (MoveSelectionAtPoint(pt)) {
			UINT nNewSelStart = GetSelectionStart();
			UINT nNewSelEnd = GetSelectionEnd();

			auto bl = true;
			if (nNewSelStart < m_selDragStart)
				bl = false;

			MoveSelection(min(m_selDragStart, nNewSelStart), max(m_selDragStart, nNewSelEnd), bl); // (nNewSelStart>_uSelDragStart));
			InvalidateRect();
		}
	}
}


BOOL D2DTextbox::Render(D2DContext& cxt, const d2dw::CRectF& rc, LPCWSTR psz, int nCnt, int nSelStart, int nSelEnd, bool bTrail,
	const COMPOSITIONRENDERINFO *pCompositionRenderInfo, UINT nCompositionRenderInfo)
{
	cxt.pWindow->m_pDirect->DrawTextLayout(*(cxt.pWindow->m_spProp->Format), psz, d2dw::CRectF(rc.left, rc.top, rc.right, rc.bottom));

	// Render selection,caret
	d2dw::CRectF rcSelCaret(rc.left, rc.top, rc.left + 1.0f, rc.top + (float)nLineHeight_);

	CaretRect xcaret(bTrail);

	if (nLineCnt_) {
#pragma region CaretRectX

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
						cxt.pWindow->m_pDirect->FillSolidRectangle(d2dw::SolidFill(d2dw::CColorF(0, 140.f / 255, 255.f / 255, 100.f / 255)), m_lineInfos[r].CharInfos[j].rc);
					}

					bool blast = bTrail;

					if (blast) {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelEndInLine - 1].rc;
						rcSelCaret.left = rcSelCaret.right;
					} else {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left;
					}
					rcSelCaret.right++;

					xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine);


				} else // caret, 範囲指定されてない場合
				{
					if (nSelStartInLine == m_lineInfos[r].nCnt && !m_lineInfos[r].CharInfos.empty()) {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left + 1;

						_ASSERT(rcSelCaret.bottom >= 0);
						_ASSERT(rcSelCaret.bottom < 65000);

						xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine);


					} else if (nSelStartInLine > -1 && !m_lineInfos[r].CharInfos.empty()) {
						rcSelCaret = m_lineInfos[r].CharInfos[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left + 1;

						_ASSERT(rcSelCaret.bottom >= 0);
						xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine);

					}
				}

			}

			// キャレット表示位置が確定
			if (xcaret.IsComplete(r)) {
				if (bTrail)
					break;

			}

			for (UINT j = 0; j < nCompositionRenderInfo; j++) {

				if ((pCompositionRenderInfo[j].nEnd >= m_lineInfos[r].nPos) &&
					(pCompositionRenderInfo[j].nStart <= m_lineInfos[r].nPos + m_lineInfos[r].nCnt)) {
					UINT nCompStartInLine = 0;
					UINT nCompEndInLine = m_lineInfos[r].nCnt;
					int  nBaseLineWidth = (nLineHeight_ / 18) + 1;

					if (pCompositionRenderInfo[j].nStart > m_lineInfos[r].nPos)
						nCompStartInLine = pCompositionRenderInfo[j].nStart - m_lineInfos[r].nPos;

					if (pCompositionRenderInfo[j].nEnd < m_lineInfos[r].nPos + m_lineInfos[r].nCnt)
						nCompEndInLine = pCompositionRenderInfo[j].nEnd - m_lineInfos[r].nPos;

					for (UINT k = nCompStartInLine; k < nCompEndInLine; k++) {
						UINT uCurrentCompPos = m_lineInfos[r].nPos + k - pCompositionRenderInfo[j].nStart;
						BOOL bClause = FALSE;

						if (k + 1 == nCompEndInLine) {
							bClause = TRUE;
						}

						if ((pCompositionRenderInfo[j].da.crText.type != TF_CT_NONE) &&
							(pCompositionRenderInfo[j].da.crBk.type != TF_CT_NONE)) {
							int a = 0;
						}

						if (pCompositionRenderInfo[j].da.lsStyle != TF_LS_NONE) {
							// 変換途中の下線の描画
							{
								d2dw::CRectF rc = m_lineInfos[r].CharInfos[k].rc;

								d2dw::CPointF pts[2];
								pts[0].x = rc.left;
								pts[0].y = rc.bottom;
								pts[1].x = rc.right - (bClause ? nBaseLineWidth : 0);
								pts[1].y = rc.bottom;
								cxt.pWindow->m_pDirect->DrawSolidLine(*(cxt.pWindow->m_spProp->Line), pts[0], pts[1]);
							}
						}
					}
				}
			}
		}
#pragma endregion


		if (xcaret.empty()) {
			//次行の文字なし先頭列
			rcSelCaret.left = rc.left;
			rcSelCaret.right = rc.left + 1;

			rcSelCaret.OffsetRect(0, nLineHeight_*nLineCnt_);
			xcaret.Push(rcSelCaret, nLineCnt_, 0, 0);
		}


		// 情報
		int CaretRow = xcaret.row() + 1;
		int CaretCol = xcaret.col();
		int LineNumber = nLineCnt_;


		// Caret表示
		DrawCaret(cxt, xcaret.Get());

	} else {
		// 文字がない場合Caret表示
		DrawCaret(cxt, rcSelCaret);
	}

	return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

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
				int nWidth = m_lineInfos[r].CharInfos[j].GetWidth();
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


//----------------------------------------------------------------
//
//
// Calc layout 行数の把握と文字単位にPOS,LEN,RECTを取得
//----------------------------------------------------------------

BOOL D2DTextbox::Layout(D2DContext& cxt, const WCHAR *psz, int nCnt, const d2dw::CSizeF& sz, int zCaret, int& StarCharPos)
{
	LPCWSTR str = psz;
	nLineCnt_ = 1;
	StarCharPos_ = 0;

	// 行数を計算

	nLineCnt_ = 0;

	{
		BOOL bNewLine = TRUE;
		for (int i = 0; i < nCnt; i++) {
			switch (psz[i]) {
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

		for (int i = 0; i < nCnt; i++) {
			switch (psz[i]) {
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


				xassert(crlf == 0 || crlf == psz[i]); // CRLFはだめ、CRCR.. or LFLF..はOK
				crlf = psz[i];
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

		StarCharPos_ = 0;
		int slen = nCnt;

		// 文字文のRECT取得
		std::vector<d2dw::CRectF> charRects = cxt.pWindow->m_pDirect->CalcCharRects(*(cxt.pWindow->m_spProp->Format), std::wstring(psz).substr(StarCharPos), sz);
		if (charRects.empty()) {
			auto pLayout = cxt.pWindow->m_pDirect->GetTextLayout(*(cxt.pWindow->m_spProp->Format), std::wstring(psz).substr(StarCharPos), sz);
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
						rc.OffsetRect(2.5, 2.5);
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
					rc.OffsetRect(2.5, 2.5);
					m_lineInfos[r].CharInfos[0].rc = rc;

					xassert(m_lineInfos[r].nCnt == 0);
				}
			}
		}

		if (nCnt) {
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

