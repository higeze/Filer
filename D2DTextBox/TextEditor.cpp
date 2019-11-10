#include "text_stdafx.h"
#include "TextEditor.h"
#include "d2dwindowcontrol.h"
#include "TextStoreACP.h"
#include "D2DWindow.h"
#include "CellProperty.h"
#include "Direct2DWrite.h"
#include "Debug.h"

#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
extern ITfThreadMgr2* g_pThreadMgr;
#else
extern ITfThreadMgr* g_pThreadMgr;
#endif
extern TfClientId g_TfClientId;

using namespace TSF;

CTextEditor::CTextEditor(D2DTextbox* pTxtbox) 
	:m_pTxtbox(pTxtbox)
{
	pCompositionRenderInfo_ = NULL;
    nCompositionRenderInfo_ = 0;
}

CTextEditor::~CTextEditor() 
{
    UninitTSF();
}

//----------------------------------------------------------------
//
// Application Initialize
//
//----------------------------------------------------------------

void CTextEditor::InitTSF()
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
	if (FAILED(g_pThreadMgr->AssociateFocus(m_pTxtbox->m_pWnd->m_hWnd, m_pDocumentMgr, &pDocumentMgrPrev))) {
		throw std::exception(FILE_LINE_FUNC);
	}
#endif

	m_pTextEditSink = CComPtr<CTextEditSink>(new CTextEditSink(this));
	if (!m_pTextEditSink) { throw std::exception(FILE_LINE_FUNC); }
	 m_pTextEditSink->_Advise(m_pInputContext);
}

void CTextEditor::UninitTSF()
{
    if (m_pTextEditSink)
    {
        m_pTextEditSink->_Unadvise();
    }
	
	if (m_pDocumentMgr)
    {
        m_pDocumentMgr->Pop(TF_POPF_ALL);
	}
}

void CTextEditor::MoveSelection(int nSelStart, int nSelEnd, bool bTrail)
{
	if ( nSelEnd < nSelStart )
		std::swap( nSelStart, nSelEnd );

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

void CTextEditor::MoveSelectionNext()
{
    int nTextLength = (int)m_text.size();

	int zCaretPos = (m_isSelTrail ? m_selEnd : m_selStart );
	zCaretPos = min(nTextLength, zCaretPos+1); // 1:次の文字

    m_selStart = m_selEnd = zCaretPos;
    m_pTextStore->OnSelectionChange();
}

void CTextEditor::MoveSelectionPrev()
{
	int zCaretPos = (m_isSelTrail ? m_selEnd : m_selStart );
	zCaretPos = max(0, zCaretPos-1);

    m_selEnd = m_selStart = zCaretPos;
    m_pTextStore->OnSelectionChange();
}

BOOL CTextEditor::MoveSelectionAtPoint(const CPoint& pt)
{
    BOOL bRet = FALSE;
    int nSel = (int)layout_.CharPosFromPoint(m_pTxtbox->m_pWnd->m_pDirect->Pixels2Dips(pt));
    if (nSel != -1)
    {
        MoveSelection(nSel, nSel,true);
        bRet = TRUE;
    }
    return bRet;
}

BOOL CTextEditor::MoveSelectionAtNearPoint(const CPoint& pt)
{
    BOOL bRet = FALSE;
    int nSel = (int)layout_.CharPosFromNearPoint(m_pTxtbox->m_pWnd->m_pDirect->Pixels2Dips(pt));
    if (nSel != -1)
    {
        MoveSelection(nSel, nSel,true);
        bRet = TRUE;
    }
    return bRet;
}

BOOL CTextEditor::MoveSelectionUpDown(BOOL bUp, bool bShiftKey )
{
    d2dw::CRectF rc;

	if ( bUp )
	{
		if (!layout_.RectFromCharPos(m_selStart, &rc))
			return FALSE;
	}
	else
	{
		if (!layout_.RectFromCharPos(m_selEnd, &rc))
		{
			//MoveSelectionNext();
			return false;
		}
			
	}

    POINT pt;
    pt.x = rc.left;
    if (bUp)
    {
        pt.y = rc.top - ((rc.bottom - rc.top) / 2);
        if (pt.y < 0)
            return FALSE;
    }
    else
    {
        pt.y = rc.bottom + ((rc.bottom - rc.top) / 2);
    }

	UINT nSel = bUp ? m_selEnd : m_selStart ;

	// 文字間の場合
	if ( MoveSelectionAtPoint(pt) ) 
	{				
		if ( bShiftKey )
		{
			UINT nSel2 = bUp ? m_selStart : m_selEnd ;
			MoveSelection(nSel, nSel2,!bUp );
		}
		
		return TRUE; 
	}

	// 行の最後のさらに後ろ側の場合
	if ( MoveSelectionAtNearPoint(pt) )
	{
		if ( bShiftKey )
		{
			UINT nSel2 = bUp ? m_selStart : m_selEnd ;
			MoveSelection(nSel, nSel2,!bUp );
		}
		
		return TRUE; 
	}
	
	return FALSE;
}

BOOL CTextEditor::MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey)
{
    BOOL bRet = FALSE;
    UINT nSel,nSel2;

    if (bFirst)
    {
		// when pushed VK_HOME
		m_startCharPos = 0;
		nSel2 = m_selEnd;
        nSel = layout_.FineFirstEndCharPosInLine(m_selStart, TRUE);
    }
    else
    {
        // when pushed VK_END
		m_startCharPos = 0;
		nSel2 = m_selStart;
		nSel = layout_.FineFirstEndCharPosInLine(m_selEnd, FALSE);
    }

    if (nSel != (UINT)-1)
    {
		if ( bShiftKey )
		{
			MoveSelection(nSel, nSel2,true);
		}
		else
		{
			MoveSelection(nSel, nSel);
		}

        bRet = TRUE;
    }

	
    return bRet;
}

void CTextEditor::InvalidateRect()
{
	layout_.bRecalc_ = true;
}

BOOL CTextEditor::InsertAtSelection(LPCWSTR psz)
{
	layout_.bRecalc_ = true;

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

BOOL CTextEditor::DeleteAtSelection(BOOL fBack)
{
	layout_.bRecalc_ = true;

    if (!fBack && (m_selEnd < (int)m_text.size()))
    {
		m_text.erase(m_selEnd, 1);

		
		LONG ecs = m_selEnd;

        m_pTextStore->OnTextChange(ecs, ecs + 1, ecs);
		OnTextChange(m_text);
		m_changed(m_text);

    }
	 
    if (fBack && (m_selStart > 0))
    {
		m_text.erase(m_selStart - 1, 1);

        m_selStart--;
        m_selEnd = m_selStart;

		LONG acs = m_selStart;
        m_pTextStore->OnTextChange(acs, acs + 1, acs );
		OnTextChange(m_text);
		m_changed(m_text);
        m_pTextStore->OnSelectionChange();
    }

    return TRUE;
}

BOOL CTextEditor::DeleteSelection()
{
	layout_.bRecalc_ = true;

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

void CTextEditor::OnTextChange(const std::wstring& text)
{
	d2dw::CRectF rcClient(m_pTxtbox->GetClientRect());
	d2dw::CRectF rcContent(m_pTxtbox->GetContentRect());

	d2dw::CSizeF szNewContent(m_pTxtbox->m_pWnd->m_pDirect->CalcTextSizeWithFixedWidth(*(m_pTxtbox->m_pProp->Format), text, rcContent.Width()));
	d2dw::CRectF rcNewContent(szNewContent);
	rcNewContent.InflateRect(*(m_pTxtbox->m_pProp->Padding));
	rcNewContent.InflateRect(m_pTxtbox->m_pProp->Line->Width*0.5f);
	CRect rcNewClient(m_pTxtbox->m_pWnd->m_pDirect->Dips2Pixels(rcNewContent));

	if (rcNewClient.Height() > rcClient.Height()) {
		CRect rc;
		::GetWindowRect(m_pTxtbox->m_pWnd->m_hWnd, &rc);
		CPoint pt(rc.TopLeft());
		::ScreenToClient(::GetParent(m_pTxtbox->m_pWnd->m_hWnd), &pt);
		::MoveWindow(m_pTxtbox->m_pWnd->m_hWnd, pt.x, pt.y, rc.Width(), rcNewClient.Height(), TRUE);
	}

}

int CTextEditor::CurrentCaretPos()
{
	return (m_isSelTrail ? m_selEnd : m_selStart );
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::Render(D2DContext& cxt )
{	
	int zCaretPos = CurrentCaretPos(); //(ct_->bSelTrail_ ? m_selEnd : m_selStart );

	if ( layout_.bRecalc_ )
	{

		layout_.Layout(cxt, m_text.c_str(), m_text.size(), m_pTxtbox->GetContentRect().Size(), m_isSingleLine, zCaretPos, m_startCharPos, cxt.textformat);
		layout_.bRecalc_ = false;
	}
		
	int selstart = (int)m_selStart - m_startCharPos;
	int selend = (int)m_selEnd - m_startCharPos;

	layout_.Render(cxt, m_pTxtbox->GetContentRect() , m_text.c_str(), m_text.size(), selstart, selend,m_isSelTrail,pCompositionRenderInfo_, nCompositionRenderInfo_);
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::CalcRender(D2DContext& cxt )
{
	int x = 0;
	
	//::OutputDebugStringA((boost::format("TextBuff:%1%, TextLen:%2%\r\n") % ct_->GetTextBuffer() % ct_->GetTextLength()).str().c_str());
	d2dw::CSizeF size(m_pTxtbox->GetContentRect().Size());
	layout_.Layout(cxt, m_text.c_str(), m_text.size(), size, m_isSingleLine,0, x, cxt.textformat);	
	layout_.bRecalc_ = false;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------



void CTextEditor::SetFocus()
{
    if (m_pDocumentMgr)
    {
		g_pThreadMgr->SetFocus(m_pDocumentMgr);
    }


}


//----------------------------------------------------------------
//
// 文字エリアの実設定
//
//----------------------------------------------------------------
void CTextEditor::Reset( IBridgeTSFInterface* bi )
{ 	
	bri_ = bi;
	
	if ( bi )
	{
		//FRectFBoxModel rc1 = bi->GetClientRectEx();
		//FRectF rc = rc1.GetContentRect();// .ZeroRect();

		//ct_->rc_ = rc;

		//ct_->rc_.left = 0;
		//ct_->rc_.top = 0;
		//ct_->rc_.right= rc.right - rc.left; 
		//ct_->rc_.bottom= rc.bottom - rc.top;

		//ct_->view_size_.cx = (LONG)(rc.right - rc.left);
		//ct_->view_size_.cy = (LONG)(rc.bottom - rc.top);

		
	}
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::ClearCompositionRenderInfo()
{
    if (pCompositionRenderInfo_)
    {
        LocalFree(pCompositionRenderInfo_);
        pCompositionRenderInfo_ = NULL;
        nCompositionRenderInfo_ = 0;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda)
{
    if (pCompositionRenderInfo_)
    {
        void *pvNew = LocalReAlloc(pCompositionRenderInfo_, 
                                   (nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO),
                                   LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!pvNew)
            return FALSE;

        pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)pvNew;
    }
    else
    {
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
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//void CTextEditorCtrl::SetContainer( IBridgeTSFInterface* rect_size )
//{
//	//CTextEditor::SetContainer(ct);
//
//	Reset(rect_size);
//
//}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::Create()
{	
	InitTSF();

	m_pTextEditSink->OnChanged_ = std::bind(&CTextEditorCtrl::OnEditChanged, this );// &CTextEditorCtrl::OnChanged;
}
void CTextEditorCtrl::OnEditChanged()
{
	

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

LRESULT CTextEditorCtrl::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0;

    switch (message)
    {
        case WM_PAINT:
               OnPaint(d->cxt_);
		break;
        /*case WM_SETFOCUS:
            focusは別系統で
			this->OnSetFocus(wParam, lParam);
		break;*/
        case WM_KEYDOWN:
            if ( this->OnKeyDown(wParam, lParam) )
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

			if ( heldControl )
				return 0;
            
			// normal charcter input. not TSF.
			if ( wch >= L' ' ||  (wch == L'\r'&& !m_isSingleLine )) 
            {				
				if ( wch < 256 )
				{
					WCHAR wc[] = { wch, '\0'};
		            this->InsertAtSelection(wc);
			        InvalidateRect();
				}
				else
				{
					// outof input
//					TRACE( L"out of char %c\n", wch );
				}
            }
		break;
	}
    return ret;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnPaint(D2DContext& hdc)
{
    Render(hdc);
}

//----------------------------------------------------------------
// 
//
//
//----------------------------------------------------------------

BOOL CTextEditorCtrl::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    BOOL ret = true; 

	bool heldShift   = (GetKeyState(VK_SHIFT)& 0x80) != 0;

	int nSelStart;
    int nSelEnd;
    switch (0xff & wParam)
    {
        case VK_LEFT:
             if (heldShift)
             {                 				 
				 nSelStart = GetSelectionStart();
                 nSelEnd = GetSelectionEnd();
                 if (nSelStart > 0)
                 {					
					if ( nSelStart == nSelEnd )
						m_isSelTrail = false;

					if ( m_isSelTrail )
						MoveSelection(nSelStart, nSelEnd-1, true );
					else
						MoveSelection(nSelStart - 1, nSelEnd, false);											
                 }
             }
             else
             {
                 MoveSelectionPrev();
             }
		break;

        case VK_RIGHT:
             if (heldShift)
             {
                 nSelStart = GetSelectionStart();
                 nSelEnd = GetSelectionEnd();

				 if ( nSelStart == nSelEnd )
						m_isSelTrail = true;

				if ( m_isSelTrail )
					MoveSelection(nSelStart, nSelEnd + 1,true);
				else
					MoveSelection(nSelStart+1, nSelEnd,false);
             }
             else
             {
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
             if (nSelStart == nSelEnd)
             {
                 DeleteAtSelection(FALSE);
             }
             else
             {
                 DeleteSelection();
             }
             
		break;

        case VK_BACK:
             nSelStart = GetSelectionStart();
             nSelEnd = GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 DeleteAtSelection(TRUE);
             }
             else
             {
                 DeleteSelection();
             }
             
		break;
		case VK_ESCAPE:
			nSelEnd = GetSelectionEnd();
			MoveSelection(nSelEnd, nSelEnd);
		break;
    }

	layout_.bRecalc_ = true;

	return ret;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
    //SetFocus();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    CPoint pt((short)LOWORD(lParam), (short)(HIWORD(lParam)));
    _uSelDragStart = (UINT)-1;
    //pt.x = GET_X_LPARAM(lParam);
    //pt.y = GET_Y_LPARAM(lParam);



    if (MoveSelectionAtPoint(pt))
    {
        InvalidateRect();
        _uSelDragStart = GetSelectionStart();
    }
	else
	{
		int end = m_selEnd;
		MoveSelection( end, end, true);

	}
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    UINT nSelStart = GetSelectionStart();
    UINT nSelEnd = GetSelectionEnd();
    CPoint pt((short)LOWORD(lParam), (short)HIWORD(lParam));
 
    if (MoveSelectionAtPoint(pt))
    {
        UINT nNewSelStart = GetSelectionStart();
        UINT nNewSelEnd = GetSelectionEnd();

		auto bl = true;
			if ( nNewSelStart < _uSelDragStart )
				bl = false;

        MoveSelection(min(nSelStart, nNewSelStart), max(nSelEnd, nNewSelEnd),bl); // (nNewSelStart>_uSelDragStart));
        InvalidateRect();
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
    {
        CPoint pt((std::max)((short)0, (short)LOWORD(lParam)), (std::max)((short)0, (short)HIWORD(lParam)));
        //pt.x = max(0, GET_X_LPARAM(lParam));
        //pt.y = max(0, GET_Y_LPARAM(lParam));

        if (MoveSelectionAtPoint(pt))		
        {
            UINT nNewSelStart = GetSelectionStart();
            UINT nNewSelEnd = GetSelectionEnd();

			auto bl = true;
			if ( nNewSelStart < _uSelDragStart )
				bl = false;

            MoveSelection(min(_uSelDragStart, nNewSelStart), max(_uSelDragStart, nNewSelEnd), bl); // (nNewSelStart>_uSelDragStart));
            InvalidateRect();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
//
// CTextEditSink
//
//////////////////////////////////////////////////////////////////////////////
#include "DisplayAttribute.h"
#pragma region IUnknown
//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextEditSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTextEditSink::Release()
{
    long cr;

    cr = --_cRef;

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}
#pragma endregion
//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------
#define TES_INVALID_COOKIE  ((DWORD)(-1))

CTextEditSink::CTextEditSink(CTextEditor *pEditor)
{
    _cRef = 1;
    _dwEditCookie = TES_INVALID_COOKIE;
    _pEditor = pEditor;
}

//+---------------------------------------------------------------------------
//
// EndEdit
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    CDispAttrProps *pDispAttrProps = GetDispAttrProps();
    if (pDispAttrProps)
    {
        IEnumTfRanges *pEnum;
        if (SUCCEEDED(pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT,
                                                             pDispAttrProps->GetPropTablePointer(),
                                                             pDispAttrProps->Count(),
                                                             &pEnum)) && pEnum)
        {
            ITfRange *pRange;
            if (pEnum->Next(1, &pRange, NULL) == S_OK)
            {
                // We check if there is a range to be changed.
                pRange->Release();

                _pEditor->ClearCompositionRenderInfo();

                // We read the display attribute for entire range.
                // It could be optimized by filtering the only delta with ITfEditRecord interface. 
                ITfRange *pRangeEntire = NULL;
                ITfRange *pRangeEnd = NULL;
                if (SUCCEEDED(pic->GetStart(ecReadOnly, &pRangeEntire)) &&
                    SUCCEEDED(pic->GetEnd(ecReadOnly, &pRangeEnd)) &&
                    SUCCEEDED(pRangeEntire->ShiftEndToRange(ecReadOnly, pRangeEnd, TF_ANCHOR_END)))
                {
                    IEnumTfRanges *pEnumRanges;
                    ITfReadOnlyProperty *pProp = NULL;

                    GetDisplayAttributeTrackPropertyRange(ecReadOnly, pic, pRangeEntire, &pProp, pDispAttrProps);

                    if (SUCCEEDED(pProp->EnumRanges(ecReadOnly, &pEnumRanges, pRangeEntire)))
                    {
                        while (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
                        {
                            TF_DISPLAYATTRIBUTE da;
                            TfGuidAtom guid;
                            if (GetDisplayAttributeData(ecReadOnly, pProp, pRange, &da, &guid) == S_OK)
                            {
                                ITfRangeACP *pRangeACP;
                                if (pRange->QueryInterface(IID_ITfRangeACP, (void **)&pRangeACP) == S_OK)
                                {
                                    LONG nStart;
                                    LONG nEnd;
                                    pRangeACP->GetExtent(&nStart, &nEnd);
                                    
									
									_pEditor->AddCompositionRenderInfo(nStart, nStart + nEnd, &da);


                                    pRangeACP->Release();
                                }
                            }
                        }
                    }
                }

                if (pRangeEntire)
                    pRangeEntire->Release();
                if (pRangeEnd)
                    pRangeEnd->Release();
 
            }
            pEnum->Release();
        }

        delete pDispAttrProps;
    }

	if ( OnChanged_ )
		OnChanged_();

	

    return S_OK;
}

#pragma region Advise_Unadvice
//+---------------------------------------------------------------------------
//
// CTextEditSink::Advise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Advise(ITfContext *pic)
{
    HRESULT hr;
    ITfSource *source = NULL;

    _pic = NULL;
    hr = E_FAIL;

    if (FAILED(pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwEditCookie)))
        goto Exit;

    _pic = pic;
    _pic->AddRef();

    hr = S_OK;

Exit:
    if (source)
        source->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// CTextEditSink::Unadvise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Unadvise()
{
    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_pic == NULL)
        goto Exit;

    if (FAILED(_pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->UnadviseSink(_dwEditCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    if (source)
        source->Release();

    if (_pic)
    {
        _pic->Release();
        _pic = NULL;
    }

    return hr;
}
#pragma endregion