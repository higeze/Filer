#include "text_stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "Selection.h"

#include <Shellapi.h>

#include "MoveTarget.h"
#include <random>

#include "Direct2DWrite.h"
#include "CellProperty.h"
#define CLASSNAME L"D2DWindow"

using namespace V4;

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
			
			d->m_pDirect->BeginDraw();

			d->m_pDirect->GetHwndRenderTarget()->Clear(d2dw::CColorF(1.f,1.f,1.f));

			d->WndProc( message, wParam, lParam ); // All objects is drawned.

			if (d->redraw_)
			{
				InvalidateRect(hWnd, NULL, FALSE);
				d->redraw_ = 0;
			}

			d->m_pDirect->EndDraw();

			EndPaint(hWnd, &ps);
			return 0;
		}
		break;
		case WM_ERASEBKGND:
			return 1;
		break;
		case WM_SIZE :
		{
			d->m_pDirect->GetHwndRenderTarget()->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
			d->WndProc( message, wParam,lParam );
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

		case WM_MOUSEWHEEL:// マウスホイール	
		{				
			CPoint pt(LOWORD(lParam), HIWORD(lParam));			
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
}

HWND D2DWindow::CreateD2DWindow( DWORD dwWSEXSTYLE, HWND parent, DWORD dwWSSTYLE, RECT rc, UINT* img_resource_id, int img_cnt )
{
	_ASSERT( dwWSSTYLE & WS_CHILD );
	_ASSERT( parent );

	dwWSSTYLE |=WS_CLIPCHILDREN;

	HWND h = parent;
	
	m_hWnd =  ::CreateWindowExW( dwWSEXSTYLE, CLASSNAME, L"", dwWSSTYLE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, parent, NULL, ::GetModuleHandle(0), this ); 	

	cxt_.Init();
	cxt_.pWindow = this;

	cxt_.textformat = m_pDirect->GetTextFormat(*(m_spProp->Format));

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

	redraw_ = 0;

	// create textbox control
	m_pTxtbox.reset(new D2DTextbox(this, m_spProp, V4::D2DTextbox::MULTILINE, m_changed));
	m_pTxtbox->SetStat(V4::STAT::BORDER);
	m_pTxtbox->CreateWindow(this, VISIBLE, L"txtbox");
	m_pTxtbox->SetText(m_strInit.c_str());

	// OnCreateで各子コントロールを作成後にサイズの調整が必要
	SendMessage(m_hWnd, WM_SIZE,0,MAKELPARAM(rc.right-rc.left,rc.bottom-rc.top));
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
	}
	else if (message == WM_SIZE && m_pTxtbox)
	{
		m_pTxtbox->WndProc(this,message,wParam,lParam);
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
			if (m_pTxtbox) {
				m_pTxtbox->DestroyControl();
			}

			Clear();
		}
		break;		
	}
	return ret;
}
