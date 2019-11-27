#include "text_stdafx.h"
#include "D2DWindow.h"
#include "Textbox.h"

#include <Shellapi.h>

#include <random>

#include "Direct2DWrite.h"
#include "CellProperty.h"
#define CLASSNAME L"D2DWindow"

LRESULT D2DWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);

	SetFocus(m_hWnd);

	bHandled = TRUE;
	return 0;
}

LRESULT D2DWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);

	m_pDirect->BeginDraw();

	m_pDirect->GetHwndRenderTarget()->Clear(d2dw::CColorF(1.f, 1.f, 1.f));

	WndProc(uMsg, wParam, lParam); // All objects is drawned.

	if (redraw_)
	{
		InvalidateRect(m_hWnd, NULL, FALSE);
		redraw_ = 0;
	}

	m_pDirect->EndDraw();

	EndPaint(m_hWnd, &ps);
	return 0;
}

LRESULT D2DWindow::OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT D2DWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
	WndProc(uMsg, wParam, lParam);
	bHandled = FALSE;
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

LRESULT D2DWindow::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WndProc(uMsg, wParam, lParam);

	if (redraw_)
	{
		InvalidateRect(m_hWnd, NULL, FALSE);
		redraw_ = 0;
	}
	bHandled = FALSE;
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

LRESULT D2DWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return WndProc(uMsg, wParam, lParam);
}




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
			d->m_hWnd = hWnd;
			BOOL bHandled;
			return d->OnCreate(message, wParam, lParam, bHandled);
		}		
		break;
		case WM_DISPLAYCHANGE:
		case WM_PAINT:
		{
			BOOL bHandled;
			return d->OnPaint(message, wParam, lParam, bHandled);
		}
		break;
		case WM_ERASEBKGND:
		{
			BOOL bHandled;
			return d->OnEraseBkGnd(message, wParam, lParam, bHandled);
		}
		break;
		case WM_SIZE :
		{
			BOOL bHandled;
			return d->OnSize(message, wParam, lParam, bHandled);
		}
		break;
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
			//case WM_IME_NOTIFY:
		case WM_IME_STARTCOMPOSITION:
		case WM_IME_COMPOSITION:
		case WM_IME_ENDCOMPOSITION:
		case WM_KILLFOCUS: // 0x8
		case WM_KEYDOWN: // 0x100
		case WM_KEYUP: //0x101
		{
			BOOL bHandled;
			return d->OnChar(message, wParam, lParam, bHandled);
		}
		break;
		
		case WM_DESTROY:
		{
			BOOL bHandled;
			return d->OnDestroy(message, wParam, lParam, bHandled);
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

D2DWindow::~D2DWindow() {}

HWND D2DWindow::CreateD2DWindow( DWORD dwWSEXSTYLE, HWND parent, DWORD dwWSSTYLE, RECT rc, UINT* img_resource_id, int img_cnt )
{
	_ASSERT( dwWSSTYLE & WS_CHILD );
	_ASSERT( parent );

	dwWSSTYLE |=WS_CLIPCHILDREN;

	HWND h = parent;
	
	m_hWnd =  ::CreateWindowExW( dwWSEXSTYLE, CLASSNAME, L"", dwWSSTYLE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, parent, NULL, ::GetModuleHandle(0), this );

	redraw_ = 0;

	// create textbox control
	m_pTxtbox.reset(new D2DTextbox(this, m_strInit, m_spProp, m_changed));

	// OnCreateで各子コントロールを作成後にサイズの調整が必要
	SendMessage(m_hWnd, WM_SIZE,0,MAKELPARAM(rc.right-rc.left,rc.bottom-rc.top));
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

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			redraw_ = 1; 
		break;
	}
	return ret;
}
