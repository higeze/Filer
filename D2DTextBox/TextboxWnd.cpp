#include "text_stdafx.h"
#include <Shellapi.h>
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

	//m_pTxtbox->OnCreate(CreateEvent(this, wParam, lParam));

	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	m_pDirect->BeginDraw();

	m_pDirect->GetHwndRenderTarget()->Clear(d2dw::CColorF(1.f, 1.f, 1.f));

	m_pTxtbox->OnPaint(PaintEvent(this));
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
	m_pTxtbox->OnChar(CharEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ((wParam == VK_RETURN) && !(::GetKeyState(VK_MENU) & 0x8000)) {
		//Do not send message to children
		::SetFocus(::GetParent(m_hWnd));
	}
	else if ((wParam == VK_TAB) && !(::GetKeyState(VK_MENU) & 0x8000)) {
		//Do not send message to children
		::SetFocus(::GetParent(m_hWnd));
	}
	else if (wParam == VK_ESCAPE) {
		//Back to initial string
		m_pTxtbox->CancelEdit();
		::SetFocus(::GetParent(m_hWnd));
	}
	else {
		m_pTxtbox->OnKeyDown(KeyDownEvent(this, wParam, lParam));
	}
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_pTxtbox->OnKillFocus(KillFocusEvent(this, wParam, lParam));
	SendMessage(WM_CLOSE, NULL, NULL);
	return 0;
}

LRESULT CTextboxWnd::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnMouseMove(MouseMoveEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnLButtonDown(LButtonDownEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_pTxtbox->OnLButtonUp(LButtonUpEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

void CTextboxWnd::OnFinalMessage(HWND m_hWnd)
{
	//m_final();
	delete this;
}

CTextboxWnd::CTextboxWnd(
	std::shared_ptr<CellProperty> pProp,
	std::function<std::wstring()> getter,
	std::function<void(const std::wstring&)> setter,
	std::function<void(const std::wstring&)> changed,
	std::function<void()> final)
//	:CWnd(), m_pTxtbox(std::make_unique<D2DTextbox>(this, pProp,getter,setter,changed,final))
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

CTextboxWnd::~CTextboxWnd() = default;
