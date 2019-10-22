﻿#include "text_stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "d2dapi.h"
#include "Selection.h"

#include "gdi32.h"
#include <Shellapi.h>

#include "MoveTarget.h"
#include <random>

#include "CellProperty.h"
#define CLASSNAME L"D2DWindow"

using namespace V4;
using namespace GDI32;


std::wstring D2DWindow::appinfo_;

//namespace V4
//{
//	Selection g_selection;
//};


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	D2DWindow* d = (D2DWindow*)::GetWindowLongPtr( hWnd, GWL_USERDATA );

	switch( message )
	{
		case WM_CREATE:
		{									
			CREATESTRUCT* st = (CREATESTRUCT*)lParam;
			
			::SetWindowLongPtr( hWnd, GWL_USERDATA,(LONG) st->lpCreateParams ); // GWL_USERDATA must be set here.
			D2DWindow* d = (D2DWindow*)::GetWindowLongPtr(hWnd, GWL_USERDATA);
			
			d->m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(hWnd);

			SetFocus(hWnd);

			return 0; // DefWindowProc(hWnd, message, wParam, lParam);
		}		
		break;
		case WM_DISPLAYCHANGE:
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);					
			ID2D1RenderTarget* cxt = d->cxt_.cxt;			
			
			cxt->BeginDraw();
			//D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();


			//cxt->SetTransform(mat);
			cxt->Clear(ColorF(ColorF::White));

			d->WndProc( message, wParam, lParam ); // All objects is drawned.

			// CAPTURE OBJECTは最後に表示 
			// Comobox内のListboxなど
			//int cap_cnt = d->capture_obj_.size();
			//if ( cap_cnt )
			//{
			//	auto p = d->capture_obj_.head();
			//	for( int i = 0; i < cap_cnt; i++ )
			//	{
			//		p[i]->WndProc( d, WM_D2D_PAINT, wParam, lParam );
			//	}
			//}


			// ここがアニメーションの計算場所
			//for(auto it = d->mts_.begin(); it!=d->mts_.end(); )
			//{					
			//	bool isend;
			//	MoveTargetEx* mtx = (MoveTargetEx*)it->first;
			//	mtx->UIFire(d, isend);

			//	if (isend)
			//		d->mts_.erase(it++); // set map流のerase方法らしい			
			//	else
			//		it++;
			//}

			if (d->redraw_)
			{
				InvalidateRect(hWnd, NULL, FALSE);
				d->redraw_ = 0;
			}


			auto hr = NoThrowIfFailed(cxt->EndDraw());

			if ( D2DERR_RECREATE_TARGET == hr )
			{
				d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 0,0 ); // RenderTargetにリンクしている物はすべてリリース

				// ディスプレイの解像度を変更したり、ディスプレイ アダプターを取り外したりすると、デバイスが消失する可能性があります

				d->m_pDirect->Clear();


				d->cxt_.cxt = d->m_pDirect->GetHwndRenderTarget();
				
				//d->cxt_.CreateRenderTargetResource( d->cxt_.cxt ); //TODOTODO

				d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 1,0 );// RenderTargetに再リンク

				InvalidateRect(hWnd, NULL, FALSE);
			}
			else if ( hr != S_OK )
			{
				// 88990015 リソースが誤ったレンダターゲットを使用
				
				D2DError( hr, L"EndDraw error", __LINE__, __FILE__ );

			}

			EndPaint(hWnd, &ps);
			return 0;
		}
		break;
		case WM_ERASEBKGND:
			return 1;
		break;
		case WM_SIZE :
		{		
			// CComPtrの参照数にきをくばること
			auto bl = ( d->cxt_.cxt != NULL );		
			if ( bl )
			{
				{
					d->m_pDirect->GetHwndRenderTarget()->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
					//CComPtr<ID2D1HwndRenderTarget> trgt;
					//auto hr = d->cxt_.cxt->											
					//if ( HR(hr))				
					//	trgt->Resize( D2D1::SizeU( LOWORD(lParam), HIWORD(lParam) ));
				}

				d->WndProc( message, wParam,lParam );
			}
					
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

		case WM_MOUSEWHEEL:// マウスホイール	
		{				
			FPoint pt(lParam);			
			ScreenToClient( hWnd, &pt );				
			lParam = MAKELONG(pt.x, pt.y );
		}
		case WM_LBUTTONDOWN:		
		case WM_RBUTTONDOWN:
			SetFocus(hWnd);
		case WM_LBUTTONDBLCLK:
		case WM_CAPTURECHANGED:		
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:		
		//case WM_MOUSEHWHEEL:
		case WM_CHAR:
		{
			if ( message == WM_CHAR )
			{
				int a = 0;
			}

			d->WndProc( message, wParam,lParam );

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}
			
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		//case WM_IME_NOTIFY:
		case WM_IME_STARTCOMPOSITION:
		case WM_IME_COMPOSITION:
		case WM_IME_ENDCOMPOSITION:
		{
			d->WndProc(  message, wParam,lParam );

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}
			
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		case WM_KILLFOCUS: // 0x8
		case WM_KEYDOWN: // 0x100
		case WM_KEYUP: //0x101
		{
			d->WndProc( message, wParam,lParam );

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}
			
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		case WM_DROPFILES:
		{
			std::vector<D2D_DROPOBJECT> ar;			
			{
				HDROP hd = (HDROP)wParam;
				UINT cnt = DragQueryFile(hd,-1,nullptr,0);
				ar.resize(cnt);
				for( UINT i = 0; i < cnt; i++ )
				{
					WCHAR cb[MAX_PATH];
					POINT pt;
					DragQueryFile(hd,i,cb,MAX_PATH);
					DragQueryPoint(hd, &pt);

					D2D_DROPOBJECT it;
					it.filename = cb;
					it.pt = pt;
					ar[i] = it;
				}
				DragFinish( hd );
			}						
			d->WndProc( WM_D2D_DRAGDROP, (WPARAM)&ar,0 );
			

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}

			return 0;
		}
		break;
		
		case WM_DESTROY:
		{			
			return d->WndProc( message, wParam,lParam );
			//return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		default :
		{
			if ( message >= WM_USER )
			{
				d->WndProc(  message, wParam,lParam );
				if ( d->redraw_ )
				{
					InvalidateRect( hWnd, NULL, FALSE );				
					d->redraw_ = 0;
				}
			}
			else 
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
}
ATOM D2DWindowRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	ZeroMemory( &wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance; //::GetModuleHandle(0);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= CLASSNAME;

	return RegisterClassEx(&wcex);
}

int D2DWindow::SecurityId(bool bNew)
{
	static int sec_id;
	
	std::random_device seed_gen;
    static std::default_random_engine engine(seed_gen());
    static std::uniform_int_distribution<> dist(-999999, 999999);  
	
	if ( bNew )
		sec_id = dist(engine);
				
	return sec_id;
}

D2DWindow::D2DWindow(
	std::shared_ptr<CellProperty> spProp,
	std::function<std::wstring()> getter,
	std::function<void(const std::wstring&)> setter,
	std::function<void(const std::wstring&)> changed,
	std::function<void()> final)
	:/*capture_obj_(256),capture_pt_(256),*/m_spProp(spProp), m_getter(getter), m_setter(setter), m_changed(changed), m_final(final), m_strInit(getter())
{
	
	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(wcx);
	
	if ( !GetClassInfoEx( GetModuleHandle(0), CLASSNAME, &wcx ))	
		D2DWindowRegisterClass( GetModuleHandle(0));
}

D2DWindow::~D2DWindow() { Clear(); }

void D2DWindow::Clear()
{
	// All object are cleared.

	//while ( !capture_obj_.empty())
	//	capture_obj_.pop();
	
//	children_ = NULL;
}

HWND D2DWindow::CreateD2DWindow( DWORD dwWSEXSTYLE, HWND parent, DWORD dwWSSTYLE, RECT rc, UINT* img_resource_id, int img_cnt )
{
	_ASSERT( dwWSSTYLE & WS_CHILD );
	_ASSERT( parent );

	dwWSSTYLE |=WS_CLIPCHILDREN;

	HWND h = parent;
	
	m_hWnd =  ::CreateWindowExW( dwWSEXSTYLE, CLASSNAME, L"", dwWSSTYLE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, parent, NULL, ::GetModuleHandle(0), this ); 	

	cxt_.Init();

	cxt_.insins = new SingletonD2DInstance();
	cxt_.insins->factory = m_pDirect->GetD2D1Factory();
	cxt_.insins->wrfactory = m_pDirect->GetDWriteFactory();
	cxt_.insins->text = m_pDirect->GetTextFormat(*(m_spProp->Format));

	cxt_.cxt = m_pDirect->GetHwndRenderTarget();

	cxt_.black = m_pDirect->GetColorBrush(D2RGB(0, 0, 0));
	cxt_.white = m_pDirect->GetColorBrush(D2RGB(255, 255, 255));
	cxt_.gray = m_pDirect->GetColorBrush(D2RGB(192, 192, 192));
	cxt_.red = m_pDirect->GetColorBrush(D2RGB(255, 0, 0));
	cxt_.ltgray = m_pDirect->GetColorBrush(D2RGB(230, 230, 230));
	cxt_.bluegray = m_pDirect->GetColorBrush(D2RGB(113, 113, 130));
	cxt_.transparent = m_pDirect->GetColorBrush(D2RGBA(0, 0, 0, 0));
	cxt_.halftone = m_pDirect->GetColorBrush(D2RGBA(113, 113, 130, 100));
	cxt_.halftoneRed = m_pDirect->GetColorBrush(D2RGBA(250, 113, 130, 150));
	cxt_.tooltip = m_pDirect->GetColorBrush(D2RGBA(255, 242, 0, 255));
	cxt_.basegray = m_pDirect->GetColorBrush(D2RGBA(241, 243, 246, 255));
	cxt_.basegray_line = m_pDirect->GetColorBrush(D2RGBA(201, 203, 205, 255));
	cxt_.basetext = m_pDirect->GetColorBrush(D2RGBA(90, 92, 98, 255));

	float dashes[] = { 2.0f };

	m_pDirect->GetD2D1Factory()->CreateStrokeStyle(
		D2D1::StrokeStyleProperties(
			D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_MITER,
			10.0f,
			D2D1_DASH_STYLE_CUSTOM,
			0.0f),
		dashes, ARRAYSIZE(dashes),
		&cxt_.dot2_
	);

	float dashes2[] = { 4.0f };
	m_pDirect->GetD2D1Factory()->CreateStrokeStyle(
		D2D1::StrokeStyleProperties(
			D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_MITER,
			10.0f,
			D2D1_DASH_STYLE_CUSTOM,
			0.0f),
		dashes2, ARRAYSIZE(dashes2),
		&cxt_.dot4_
	);

	cxt_.text = cxt_.insins->text;
	cxt_.wfactory = cxt_.insins->wrfactory;

	cxt_.cxtt.line_height = 0;
	cxt_.cxtt.xoff = 0;
	cxt_.cxtt.cxt = &cxt_;
	cxt_.cxtt.wrfactory = cxt_.insins->wrfactory;

	CComPtr<IDWriteTextLayout> tl;
	cxt_.cxtt.textformat = m_pDirect->GetTextFormat(*(m_spProp->Format));
	cxt_.wfactory->CreateTextLayout(L"T", 1, cxt_.cxtt.textformat, 1000, 1000, &tl);

	DWRITE_HIT_TEST_METRICS mt;

	float y;
	tl->HitTestTextPosition(0, true, &cxt_.cxtt.xoff, &y, &mt);

	cxt_.cxtt.line_height = mt.height;

	redraw_ = 0;
//	roundpaint_obj_ = nullptr;

	// create top control
	//D2DTopControls* cs = nullptr;
	//D2DTopControls* cs = new D2DTopControls();
	//cs->CreateWindow(this, VISIBLE, L"first layer");

	//g_selection.ctrls = cs; // do nothing

	// create textbox control
	FRectF rcText(0, 0, rc.right-rc.left, rc.bottom-rc.top);
	FRectFBoxModel rcModel(rcText);
	rcModel.BoderWidth_ = m_spProp->Line->Width;
	rcModel.Margin_.Set(0.f);
	rcModel.Padding_.Set(m_spProp->Padding->left);

	m_pTxtbox.reset(new D2DTextbox(m_hWnd, V4::D2DTextbox::MULTILINE, m_changed));
	m_pTxtbox->SetStat(V4::STAT::BORDER);
	m_pTxtbox->CreateWindow(this, nullptr, rcModel, VISIBLE, L"txtbox");
	m_pTxtbox->SetText(m_strInit.c_str());
	m_pTxtbox->SetFont(m_pDirect->GetTextFormat(*(m_spProp->Format)));


	// OnCreateで各子コントロールを作成後にサイズの調整が必要
	SendMessage(m_hWnd, WM_SIZE,0,MAKELPARAM(rc.right-rc.left,rc.bottom-rc.top));
	//SetCapture(m_pTxtbox.get());
	m_pTxtbox->StatActive(true);
	return m_hWnd;

}


LRESULT D2DWindow::WndProc( UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	// WM_PAINT, WM_SIZE, WM_D2D_RESTRUCT_RENDERTARGETはCAPTUREに関わらず、すべてにSENDすること

	if ( message == WM_PAINT  && m_pTxtbox)
	{
		m_pTxtbox->WndProc(this,WM_PAINT,wParam,lParam);

		//if (roundpaint_obj_ )
		//	roundpaint_obj_->WndProc(this, message, wParam, lParam);
	}
	else if (message == WM_SIZE && m_pTxtbox)
	{
		m_pTxtbox->WndProc(this,message,wParam,lParam);
	}
	else if ( message == WM_D2D_RESTRUCT_RENDERTARGET )
	{
		if ( wParam == 0 )
		{
			if (m_pTxtbox)
				m_pTxtbox->WndProc(this,message,wParam,lParam);
		}					
		else if ( wParam == 1 )
		{

			if (m_pTxtbox)
				m_pTxtbox->WndProc(this,message,wParam,lParam);
		}
	} else if (m_pTxtbox && (message == WM_SYSKEYDOWN || message == WM_KEYDOWN ) && (wParam == VK_RETURN ) && !(::GetKeyState(VK_MENU) & 0x8000)) {
		//Do not send message to children
		::SetFocus(::GetParent(m_hWnd));
		return 0;
	} else if (m_pTxtbox && (message == WM_KEYDOWN) && (wParam == VK_TAB)) {
		//Do not send message to children
		::SetFocus(::GetParent(m_hWnd));
		return 0;
	} else if (m_pTxtbox && (message == WM_KEYDOWN) && (wParam == VK_ESCAPE)) {
		//Back to initial string
		m_pTxtbox -> SetText(m_strInit.c_str());
		::SetFocus(::GetParent(m_hWnd));//This function delete this as result
		return 0;
	} else if (message == WM_NCDESTROY) {
		m_final();
		::OutputDebugStringA("WM_NCDESTROY");
		delete this;
		return 0;
	} else if (message == WM_KILLFOCUS) {
		m_setter(m_pTxtbox->GetText());
		::OutputDebugStringA("WM_KILLFOCUS");
		SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
		auto a = 3;
		a += 4;
	} 
	//else if ( !capture_obj_.empty() && message < WM_D2D_EV_FIRST ) {
	//	auto obj1 = capture_obj_.top();
	//
	//	auto r = obj1->WndProc( this, message, wParam, lParam ); 

	//	if ( r == LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS && message == WM_LBUTTONDOWN && m_pTxtbox)
	//		r = m_pTxtbox->WndProc(this,message,wParam,lParam);

	//	
	//	if ( message==WM_MOUSEMOVE  )
	//	{		
	//		auto dropobject = (D2DControl*)obj1;

	//		auto rcxx = dropobject->GetRect();

	//		int a = WM_D2D_DROPOBJECT_SEARCH_ACCEPTOR;
	//			
	//		// dropされるobjectをどこへ落とすか
	//		for( auto& it : drag_accepters_ )
	//		{
	//			int ra = (it)->WndProc( this, WM_D2D_DROPOBJECT_SEARCH_ACCEPTOR, (WPARAM)dropobject, 0 );
	//			if ( ra )
	//				break;
	//		}
	//	}
	//}
	//else if (WM_D2D_IDLE == message && capture_obj_.empty() && death_objects_.size())
	//{
	//	ATLTRACE(L"death object removed cnt=%d\n", death_objects_.size());
	//	death_objects_.clear();
	//}
	else if (m_pTxtbox &&  message != WM_MOUSEMOVE)
	{		
		m_pTxtbox->WndProc(this,message,wParam,lParam);
	}
	else if (m_pTxtbox &&  message==WM_MOUSEMOVE )
	{		
		m_pTxtbox->WndProc(this,message,wParam,lParam);
	}
	
	switch( message )
	{		
		// 状態変化しやすい命令はデフォルトで必ずリドローさせる。WM_MOUSEMOVEは適宜、リドローさせる。
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:		
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			redraw_ = 1; 
		break;
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			redraw_ = 1; 
		break;
		case WM_DESTROY:
		{
			::OutputDebugStringA("WM_DESTROY");
			if (m_pTxtbox) {
				m_pTxtbox->DestroyControl();
				//children_ = NULL;
			}

			Clear();
		}
		break;
		
	}
	return ret;
}

//void D2DWindow::SetCapture(D2DCaptureObject* p,  FPointF* pt, D2DMat* mat )
//{
//	::SetFocus( m_hWnd );
//
//	if (capture_obj_.empty() )
//		capture_obj_.push(p);
//	else if ( p != capture_obj_.top())
//		capture_obj_.push(p);
//
//	if ( pt )
//		capture_pt_ = *pt; //.push(*pt);
//	else
//		capture_pt_ = (FPointF(0,0));
//
//	if ( mat )
//		capture_matrix_ = *mat;
//}
//D2DCaptureObject* D2DWindow::ReleaseCapture()
//{
//	auto p = capture_obj_.top();
//	roundpaint_obj_ = nullptr;
//	
//	capture_obj_.pop();		
//
//	HWND hWnd = ::GetCapture();
//
//	::SetFocus( hWnd );
//	redraw_ = 1;
//
//
//	return p;
//}

void V4::SetCursor( HCURSOR h )
{
	if ( h != ::GetCursor() )
	{	
		::SetCursor( h );
	}
}

DWORD D2DRGBADWORD_CONV(D2D1_COLOR_F clr)
{
	DWORD r = ROUND(clr.r * 255);
	DWORD g = ROUND(clr.g * 255);
	DWORD b = ROUND(clr.b * 255);
	DWORD a = ROUND(clr.a * 255);

	return D2DRGBADWORD(r,g,b,a );
}


CComPtr<ID2D1SolidColorBrush> D2DWindow::GetSolidColor(D2D1_COLOR_F clr)
{
	return m_pDirect->GetColorBrush(d2dw::CColorF(clr.r, clr.g, clr.b, clr.a));
}
////////////////////////////////////////////////////////////////////////////////////////////
CHDL ControlHandle::handle_ = 800; // initial value

CHDL ControlHandle::CreateControlHandle(D2DControl* ctrl)
{	
	handle_++;

	m1_[handle_] = ctrl;
	m2_[ctrl->GetName()] = handle_;
	return handle_;
}
void ControlHandle::DeleteControlHandle( CHDL id )
{
	if ( id == 0 || m1_.empty() ) return;

	xassert( m1_.find(id) != m1_.end());

	D2DControl* ctrl = m1_[id];
		
	m1_[id] = NULL;
	m2_[ctrl->GetName()] = 0;

}
D2DControl* ControlHandle::GetFromControlHandle(CHDL id )
{
	if ( m1_.find(id) != m1_.end())
		return m1_[id];

	return NULL;
}
D2DControl* ControlHandle::GetFromControlName(LPCWSTR nm )
{
	if ( m2_.find(nm) != m2_.end())
	{
		return GetFromControlHandle(m2_[nm]);
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
