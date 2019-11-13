#include "text_stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "TextEditor.h"
#include "CellProperty.h"
#include "MyClipboard.h"
#include "IBridgeTSFInterface.h"


#define TAB_WIDTH_4CHAR 4

D2DTextbox::D2DTextbox(D2DWindow* pWnd, const std::shared_ptr<CellProperty>& pProp, std::function<void(const std::wstring&)> changed)
	:m_pProp(pProp), m_changed(changed)
{
	m_pWnd = pWnd;
	ctrl_ = new CTextEditor(this);
	ctrl_->Create();
	ctrl_->bri_ = this;
	ctrl_->m_changed = m_changed;
}

void D2DTextbox::CreateWindow(D2DWindow* parent, int stat, LPCWSTR name)
{
	D2DControl::CreateWindow( parent,stat,name);
	SetText(L"");
}

LRESULT D2DTextbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if ( !IsThroughMessage(message))
		return 0;

	switch ( message )
	{
		case WM_PAINT:
		{
			//PaintBackground
			d->m_pDirect->FillSolidRectangle(*(d->m_spProp->NormalFill), GetClientRect());
			//PaintLine(e.Direct, rcClient)
			d->m_pDirect->GetHwndRenderTarget()->DrawRectangle(GetClientRect(), d->m_pDirect->GetColorBrush(d->m_spProp->EditLine->Color), d->m_spProp->Line->Width);
			//PaintContent(e.Direct, rcContent);
			ctrl_->WndProc(d, WM_PAINT, wParam, lParam);
			d->redraw_ = 1;
		}
		break;
		default :
		{	
			if ( WM_D2D_APP_USER <= message )
			{
			}

			if ( (stat_ & READONLY) || (stat_ & DISABLE) ) 
				return 0;
			
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

			ret = ctrl_->WndProc( d, message, wParam, lParam ); // WM_CHAR,WM_KEYDOWNの処理など

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

	LPCWSTR s = ctrl_->m_text.c_str();
	int pos = (std::max)(0, ctrl_->m_selEnd - 1 );

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
			if ( ctrl_->m_selEnd > ctrl_->m_selStart )
			{
				ctrl_->m_text.erase( ctrl_->m_selStart, ctrl_->m_selEnd-ctrl_->m_selStart );
				ctrl_->m_selEnd = ctrl_->m_selStart;
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
		int SelLen = ctrl_->m_selEnd-ctrl_->m_selStart;
		if (SelLen > 0 )
		{
			std::wstring strCopy = ctrl_->m_text.substr(ctrl_->m_selStart, SelLen);

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

				UINT s1 = ctrl_->m_selStart;
				UINT e1 = ctrl_->m_selEnd;


				//			UINT rcnt;
				ctrl_->m_text.insert(ctrl_->m_selEnd, s1b);
				ctrl_->m_selEnd += s1b.length();
				ctrl_->m_selStart = ctrl_->m_selEnd;

				GlobalUnlock(h);


				// 上書された分を削除
				if (e1 - s1 > 0)
					ctrl_->m_text.erase(s1, e1 - s1);

				ctrl_->CalcRender(m_pWnd->cxt_);
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
	ctrl_->m_text.insert(pos, str, strlen);
	ctrl_->CalcRender(m_pWnd->cxt_);

	return 0;
}


void D2DTextbox::SetText(LPCWSTR str1)
{
	ctrl_->m_text = str1;
	ctrl_->m_selStart = 0;
	ctrl_->m_selEnd = ctrl_->m_text.size();;
	ctrl_->CalcRender(m_pWnd->cxt_);
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
	return ctrl_->m_text;  // null terminate	
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
