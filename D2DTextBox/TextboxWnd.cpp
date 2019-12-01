#include "text_stdafx.h"
#include <Shellapi.h>
#include <random>

#include "TextboxWnd.h"
#include "Textbox.h"
#include "Direct2DWrite.h"
#include "CellProperty.h"
#include "UIElement.h"
#include "ResourceIDFactory.h"

LRESULT CTextboxWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetFocus();
	// create direct
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);
	// create textbox control
	m_pTxtbox.reset(new D2DTextbox(this, m_strInit, m_spProp, m_changed));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	m_pDirect->BeginDraw();

	m_pDirect->GetHwndRenderTarget()->Clear(d2dw::CColorF(1.f, 1.f, 1.f));

	m_pTxtbox->OnPaint(PaintEvent(m_pDirect.get()));
	m_pDirect->EndDraw();

	::EndPaint(m_hWnd, &ps);
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT CTextboxWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnChar(CharEvent(m_pDirect.get(), wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ((wParam == VK_RETURN) && !(::GetKeyState(VK_MENU) & 0x8000)) {
		//Do not send message to children
		::SetFocus(::GetParent(m_hWnd));
	}
	else if (wParam == VK_TAB) {
		//Do not send message to children
		::SetFocus(::GetParent(m_hWnd));
	}
	else if (wParam == VK_ESCAPE) {
		//Back to initial string
		m_pTxtbox->SetText(m_strInit.c_str());
	}
	else {
		m_pTxtbox->OnKeyDown(KeyDownEvent(m_pDirect.get(), wParam, lParam));
	}
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_setter(m_pTxtbox->GetText());
	SendMessage(WM_CLOSE, NULL, NULL);
	return 0;
}

LRESULT CTextboxWnd::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnMouseMove(MouseMoveEvent(m_pDirect.get(), wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnLButtonDown(LButtonDownEvent(m_pDirect.get(), wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnLButtonUp(LButtonUpEvent(m_pDirect.get(), wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

void CTextboxWnd::OnFinalMessage(HWND m_hWnd)
{
	m_final();
	delete this;
}



//static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	D2DWindow* d = (D2DWindow*)::GetWindowLongPtr( hWnd, GWL_USERDATA );
//	LRESULT ret = 0;
//	BOOL bHandled = TRUE;
//	
//	switch( message )
//	{
//		case WM_CREATE:
//		{			
//			CREATESTRUCT* st = (CREATESTRUCT*)lParam;
//			
//			::SetWindowLongPtr( hWnd, GWL_USERDATA,(LONG) st->lpCreateParams ); // GWL_USERDATA must be set here.
//			D2DWindow* d = (D2DWindow*)::GetWindowLongPtr(hWnd, GWL_USERDATA);
//			d->m_hWnd = hWnd;
//			ret = d->OnCreate(message, wParam, lParam, bHandled);
//		}		
//		break;
//		case WM_DISPLAYCHANGE:
//		case WM_PAINT:
//		{
//			ret = d->OnPaint(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_ERASEBKGND:
//		{
//			ret = d->OnEraseBkGnd(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_SIZE :
//		{
//			ret = d->OnSize(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_MOUSEMOVE:
//		{
//			ret = d->OnMouseMove(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_KILLFOCUS:
//		{
//			ret = d->OnKillFocus(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_KEYDOWN:
//		case WM_SYSKEYDOWN:
//		{
//			ret = d->OnKeyDown(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_CHAR:
//		{
//			ret = d->OnChar(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_LBUTTONDOWN:
//		{
//			ret = d->OnLButtonDown(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_LBUTTONUP:
//		{
//			ret = d->OnLButtonUp(message, wParam, lParam, bHandled);
//		}
//		break;
//		case WM_RBUTTONDOWN:
//			SetFocus(hWnd);
//		case WM_LBUTTONDBLCLK:
//		case WM_CAPTURECHANGED:
//		case WM_RBUTTONUP:
//			//case WM_MOUSEHWHEEL:
//			//case WM_IME_NOTIFY:
//		case WM_IME_STARTCOMPOSITION:
//		case WM_IME_COMPOSITION:
//		case WM_IME_ENDCOMPOSITION:
//		case WM_KEYUP: //0x101
//			break;
//		//case WM_DESTROY:
//		//{
//		//	BOOL bHandled;
//		//	return d->OnDestroy(message, wParam, lParam, bHandled);
//		//}
//		break;
//		case WM_NCDESTROY:
//		{
//			BOOL bHandled;
//			ret = d->OnNCDestroy(message, wParam, lParam, bHandled);
//		}
//		break;
//		default :
//		{
//			ret = DefWindowProc(hWnd, message, wParam, lParam);
//		}
//	}
//
//	if (d && d->m_redraw)
//	{
//		InvalidateRect(hWnd, NULL, FALSE);
//		d->m_redraw = 0;
//	}
//
//	if (!bHandled) {
//		ret = DefWindowProc(hWnd, message, wParam, lParam);
//	}
//
//	return ret;
//}

//ATOM D2DWindowRegisterClass(HINSTANCE hInstance)
//{
//	WNDCLASSEX wcex;
//
//	ZeroMemory( &wcex, sizeof(wcex));
//	wcex.cbSize = sizeof(WNDCLASSEX);
//
//	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
//	wcex.lpfnWndProc	= WndProc;
//	wcex.hInstance		= hInstance; //::GetModuleHandle(0);
//	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
//	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
//	wcex.lpszClassName	= CLASSNAME;
//
//	return RegisterClassEx(&wcex);
//}

CTextboxWnd::CTextboxWnd(
	std::shared_ptr<CellProperty> spProp,
	std::function<std::wstring()> getter,
	std::function<void(const std::wstring&)> setter,
	std::function<void(const std::wstring&)> changed,
	std::function<void()> final)
	:CWnd(), m_spProp(spProp), m_getter(getter), m_setter(setter), m_changed(changed), m_final(final), m_strInit(getter())
{
	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CInplaceEditWnd"))
		.style(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground(NULL);
	CreateWindowExArgument()
		.lpszClassName(_T("CInplaceEditWnd"))
		.lpszWindowName(_T("InplaceEditWnd"))
		.dwStyle(WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"InplaceEditWnd"));

	//Add Message
	AddMsgHandler(WM_CREATE, &CTextboxWnd::OnCreate, this);
	AddMsgHandler(WM_ERASEBKGND, &CTextboxWnd::OnEraseBkGnd, this);
	AddMsgHandler(WM_SIZE, &CTextboxWnd::OnSize, this);

	AddMsgHandler(WM_PAINT, &CTextboxWnd::OnPaint, this);
	AddMsgHandler(WM_DISPLAYCHANGE, &CTextboxWnd::OnPaint, this);

	AddMsgHandler(WM_KEYDOWN, &CTextboxWnd::OnKeyDown, this);
	AddMsgHandler(WM_SYSKEYDOWN, &CTextboxWnd::OnKeyDown, this);
	AddMsgHandler(WM_CHAR, &CTextboxWnd::OnChar, this);

	AddMsgHandler(WM_KILLFOCUS, &CTextboxWnd::OnKillFocus, this);
	AddMsgHandler(WM_LBUTTONDOWN, &CTextboxWnd::OnLButtonDown, this);
	AddMsgHandler(WM_LBUTTONUP, &CTextboxWnd::OnLButtonUp, this);
	AddMsgHandler(WM_MOUSEMOVE, &CTextboxWnd::OnMouseMove, this);



}

CTextboxWnd::~CTextboxWnd() {}

//HWND D2DWindow::CreateD2DWindow( DWORD dwWSEXSTYLE, HWND parent, DWORD dwWSSTYLE, RECT rc, UINT* img_resource_id, int img_cnt )
//{
//	_ASSERT( dwWSSTYLE & WS_CHILD );
//	_ASSERT( parent );
//
//	dwWSSTYLE |=WS_CLIPCHILDREN;
//
//	HWND h = parent;
//	
//	m_hWnd =  ::CreateWindowExW( dwWSEXSTYLE, CLASSNAME, L"", dwWSSTYLE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, parent, NULL, ::GetModuleHandle(0), this );
//
//	m_redraw = 0;
//
//	return m_hWnd;
//}
