#include "text_stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "TextEditor.h"
#include "TextLayout.h"
#include "D2DCharRect.h"
#include "CellProperty.h"

//#ifdef TEXTBOXTEST
//	#include "s1.h"
//#endif

#define TAB_WIDTH_4CHAR 4

using namespace TSF;
using namespace V4;

#define LEFT_MARGIN 0.0f

LPCWSTR regexpre[] = { 
	/*
		#include <regex>
		std::wregex re(regexpre[0]);	
		bool bl = std::regex_match( L"ABCabc", re );

	*/

	L"[A-Za-z]+",									// 1文字以上 英字 
	L"[A-Za-z0-9]+",								// 1文字以上 英数字 
	L"[0-9]+",										// 1文字以上 数字 
	L"^[-+0-9][0-9]{0,14}" ,						// 1文字以上 数字１５桁まで 
	L"^[-+0-9][0-9]{0,14}[.]{1}[0-9]{1,4}",			// 1文字以上 数字　ドット １文字　小数点４桁まで
	L"[0-9]{2,4}[./ ]{1}[0-9]{1,2}[./ ]{1}[0-9]{1,4}"// 日付用 
};

D2DTextbox::D2DTextbox(D2DWindow* pWnd, const std::shared_ptr<CellProperty>& pProp, TYP typ, std::function<void(const std::wstring&)> changed)
:m_pWnd(pWnd), m_pProp(pProp), m_changed(changed)
{
	ctrl_ = new TSF::CTextEditorCtrl(this);
	ctrl_->Create(pWnd->m_hWnd);
	ctrl_->m_changed = m_changed;

	if ( typ & RIGHT || typ & CENTER || typ & VCENTER )
		typ = (D2DTextbox::TYP)(typ | TYP::SINGLELINE);

	typ_ = typ;
	bActive_ = false;
	bUpdateScbar_ = false;
	fmt_ = NULL;
	
	ct_.bSingleLine_ = ( typ_ != TYP::MULTILINE );
	ct_.LimitCharCnt_ = 65500;	

}

void D2DTextbox::CreateWindow( D2DWindow* parent, const FRectFBoxModel& rc, int stat, LPCWSTR name)
{
	D2DControl::CreateWindow( parent,rc,stat,name);
	
	if ( fmt_ )
	{
		CComPtr<IDWriteTextLayout> tl;		
		if ( S_OK == parent_->cxt_.pWindow->m_pDirect->GetDWriteFactory()->CreateTextLayout( L"T", 1, fmt_, 1000,1000,  &tl ))
		{
			DWRITE_HIT_TEST_METRICS mt;
			float y = 0, xoff=0;
			tl->HitTestTextPosition( 0, true,&xoff,&y,&mt );
			
			font_height_ = mt.height;			
		}
	}
	else
	{
		font_height_ = parent->cxt_.line_height;

	}
	SetText(L"");
}
IDWriteTextFormat* D2DTextbox::GetFormat()
{
	return fmt_;
}
void D2DTextbox::SetFont( CComPtr<IDWriteTextFormat> fmt )
{
	fmt_ = fmt;

	CalcRender(true);

}

// 文字のRECT情報を計算し、文字レイアウトを取得
void D2DTextbox::CalcRender(bool bLayoutUpdate)
{
	if ( fmt_ )
	{
		IDWriteTextFormat* old = NULL;

		old = parent_->cxt_.textformat;
		parent_->cxt_.textformat = fmt_;

		ctrl_->SetContainer( &ct_, this ); 
		//ctrl_->mat_ = mat_;	
		ctrl_->CalcRender( parent_->cxt_ );


		parent_->cxt_.textformat = old;
	}
	else
	{
		ctrl_->SetContainer( &ct_, this ); 
		ctrl_->CalcRender( parent_->cxt_ );
	}
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
			//PaintBackground(e.Direct, rcInner);
			{
				d->m_pDirect->FillSolidRectangle(*(d->m_spProp->NormalFill), GetClientRect());
			}
			//PaintLine(e.Direct, rcClient);
			{
				d->m_pDirect->GetHwndRenderTarget()->DrawRectangle(GetClientRect(), d->m_pDirect->GetColorBrush(d->m_spProp->EditLine->Color), d->m_spProp->Line->Width);
			}
			//PaintContent(e.Direct, rcContent);
			{
				ctrl_->WndProc(d, WM_PAINT, wParam, lParam);
			}
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
				
			if ( EventMessageHanler_ && EventMessageHanler_(this, message,wParam,lParam))
				return 1;

			
			switch( message )
			{
			//case WM_SIZE:
			//{
			//	//rc_.SetSize(LOWORD(lParam), HIWORD(lParam));
			//	//ctrl_->Reset(ctrl_->bri_);
			//}
			//break;
				case WM_KEYDOWN:
				{			
					ret = OnKeyDown(d,message,wParam,lParam);

					if ( ret )
						return ret;

				}
				break;

			}

			if (ctrl_->GetContainer() != &ct_ /*|| d->GetCapture() != this*/)
				return ret;

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

	LPCWSTR s = ct_.GetTextBuffer();
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
				ct_.RemoveText( ctrl_->m_selStart, ctrl_->m_selEnd-ctrl_->m_selStart );
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
			if ( OnPushKey_ )
				ret = OnPushKey_( this, (UINT)wParam );			
		break;

		default :
			if ( OnPushKey_ )			
				ret = OnPushKey_( this, (UINT)wParam );			
		break;

	}

	return ret;
}
BOOL D2DTextbox::Clipboard( HWND hwnd, TCHAR ch )
{
	if ( !OpenClipboard( hwnd ) ) return FALSE;
	
	if ( ch == L'C' ) // copy
	{
		int SelLen = ctrl_->m_selEnd-ctrl_->m_selStart;
		if ( ctrl_->GetContainer() == &ct_ &&  SelLen > 0 )
		{
			std::unique_ptr<WCHAR[]> xcb( new WCHAR[SelLen+1] );
			WCHAR* cb = xcb.get();

			ct_.GetText(ctrl_->m_selStart, cb, SelLen );
			cb[SelLen]=0;

			int cch = SelLen;
			
			HANDLE hglbCopy = GlobalAlloc(GMEM_DDESHARE, (cch + 1) * sizeof(WCHAR)); 
			if (hglbCopy == NULL) 
			{ 
				CloseClipboard(); 
				return FALSE; 
			} 
	 
			// Lock the handle and copy the text to the buffer. 
	 
			WCHAR* lptstrCopy = (WCHAR*)GlobalLock(hglbCopy); 
			memcpy(lptstrCopy, cb, cch * sizeof(WCHAR)); 
			lptstrCopy[cch] = 0; 
			GlobalUnlock(hglbCopy); 
	        
			// Place the handle on the clipboard. 
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hglbCopy); 

		}
		else
		{
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, NULL);		
		}
	}
	else if ( ch == L'V' ) // paste
	{
		HANDLE h = GetClipboardData( CF_UNICODETEXT );
		if ( h )
		{
			LPWSTR s1a = (LPWSTR)GlobalLock( h );
			auto s1b = FilterInputString( s1a, lstrlen(s1a) );

			UINT s1 = ctrl_->m_selStart;
			UINT e1 = ctrl_->m_selEnd;


			UINT rcnt;
			ct_.InsertText( ctrl_->m_selEnd, s1b.c_str(), s1b.length(), rcnt );
			ctrl_->m_selEnd += rcnt;
			ctrl_->m_selStart = ctrl_->m_selEnd;

			GlobalUnlock( h );


			// 上書された分を削除
			if ( e1 - s1 > 0 )
				ct_.RemoveText(s1, e1-s1);


			bUpdateScbar_ = true;
			CalcRender(true);
		}	
	}	
	::CloseClipboard();
	return TRUE;

}

std::wstring D2DTextbox::FilterInputString( LPCWSTR s, UINT len )
{
	if ( typ_ != TYP::MULTILINE )
	{
		for(UINT i = 0; i < len; i++ )
		{
			if ( s[i] == '\r' || s[i] == '\n' )
			{
				len = i;
				break;
			}
		}

		std::wstring r( s, len );
		return r;
	}
	else
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
}

void D2DTextbox::SetViewText(LPCWSTR str)
{
	FString org = ct_.GetTextBuffer();

	SetText(str);
	

	UINT nrCnt; 
	ct_.Clear();
	ct_.InsertText( 0, org, org.length(), nrCnt );
	ctrl_->m_selStart = ctrl_->m_selEnd = ct_.GetTextLength();
	//ct_.CaretLast();

	CalcRender(true);

}
void D2DTextbox::SetReadOnly(bool bReadOnly )
{
	if ( bReadOnly )
		stat_ |=  STAT::READONLY ;
	else
		stat_ &=  ~STAT::READONLY;
}
void D2DTextbox::SetText(VARIANT v)
{
	if ( v.vt == VT_BSTR )
		SetText( v.bstrVal );
	else
	{
		_variant_t dst;
		if ( HR(VariantChangeType( &dst, &v, 0, VT_BSTR )))
			SetText( dst.bstrVal );
	}
}
int D2DTextbox::InsertText( LPCWSTR str, int pos, int strlen)
{
	// pos < 0  is current postion.
	parent_->redraw_ = 1;

	if ( pos < 0 )
	{
		int zCaretPos = (ct_.bSelTrail_ ? ctrl_->m_selEnd : ctrl_->m_selStart );
		pos = zCaretPos;

	}
	
	if ( strlen < 0 )
		strlen = lstrlen(str);

	UINT nrCnt; 
	if ( ct_.InsertText( pos, str, strlen, nrCnt ))
	{
		ctrl_->m_selEnd += nrCnt;
		ctrl_->m_selStart += nrCnt;
	}
	bUpdateScbar_ = true;

	CalcRender(true);

	return 0;

}


void D2DTextbox::SetText(LPCWSTR str1)
{
	parent_->redraw_ = 1;
	bUpdateScbar_ = true;

	{
		FString s1b = FilterInputString( str1, lstrlen(str1) );
	
		UINT nrCnt; 
		ct_.Clear();
		ct_.InsertText( 0, s1b, s1b.length(), nrCnt );
		ctrl_->m_selStart = ctrl_->m_selEnd = ct_.GetTextLength();
	}	
	auto ct = ctrl_->GetContainer();
	auto bri = ctrl_->bri_;

	ActiveSw();

	ctrl_->SetContainer( ct, bri ); 
}

void D2DTextbox::ActiveSw()
{	
	IDWriteTextFormat* old = NULL;
	if ( fmt_ == NULL )
		fmt_ = parent_->cxt_.textformat;
	else
	{
		old = parent_->cxt_.textformat;
		parent_->cxt_.textformat = fmt_;
	}

	ctrl_->SetContainer( &ct_, this ); 
	ctrl_->CalcRender( parent_->cxt_ );

	{
		//text_layout_.Release();
		//ctrl_->GetLayout()->GetTextLayout( &text_layout_ );  // singlineの場合自動で、vcenterになる

		offpt_.y = offpt_.x = 0;

		if (typ_ & SINGLELINE) 
		{
			float topline_width = ctrl_->GetLayout()->GetLineWidth();	

			if (typ_ & RIGHT )
			{			
				offpt_.x = rc_.GetContentRect().Width() - topline_width;
			}
			else if ( typ_ & CENTER )
			{			
				offpt_.x = (rc_.GetContentRect().Width() - topline_width)/2.0f;
			}
		}

		ct_.offpt_ = offpt_;
	}

	if ( old )
		parent_->cxt_.textformat = old;
}

void D2DTextbox::StatActive( bool bActive )
{
	// captureは上位層で操作
	
	if ( bActive )
	{
		ctrl_->Password((typ_ & PASSWORD) != 0);

		CalcRender(true);
		bActive_ = true;

		ctrl_->SetFocus();
		V4::CaretActive(); // D2DContextEx.cpp

		if ( OnEnter_ )
			OnEnter_(this);
	}
	else
	{
		if ( ctrl_->GetContainer() == &ct_ )
		{						
			ctrl_->SetContainer( NULL, NULL );
		}
		V4::SetCursor( ::LoadCursor(NULL,IDC_ARROW));
		bActive_ = false;

		if ( OnLeave_ )
			OnLeave_(this);
	}
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

//FRectFBoxModel D2DTextbox::GetClientRectEx()
//{
//	// 変換ダイアログの表示位置 -> CTextStore::GetTextExt
//	FRectFBoxModel x = rc_;
//	
//
//	if ( this->typ_ == TYP::SINGLELINE || this->typ_ == TYP::PASSWORD )
//	{
//		// 26:標準フォント高で微調整
//		x.Offset(0, (rc_.Height()- 26)/2+2 ); // ???
//	}
//
//	return x;
//}
FString D2DTextbox::GetText()
{	
	return FString( ct_.GetTextBuffer());  // null terminate	
}


void D2DTextbox::SetSize( const FSizeF& sz )
{
	FRectF rc( rc_.left, rc_.top, rc_.left+sz.width, rc_.top+sz.height);
	SetRect(rc);
}
void D2DTextbox::SetRect( const FRectF& rc )
{
	rc_ = rc;
}

// Tab文字　可
void D2DTextbox::TabEnable()
{
	OnPushKey_ = [](D2DTextbox* tx,UINT key)->LRESULT
	{
		if ( key == '\t' )
		{			
			tx->InsertText( L"\t", -1, 1 );
			return 1;
		}
		return 0;
	};
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
