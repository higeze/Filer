#include "text_stdafx.h"
#include <Shellapi.h>
#include "TextboxWnd.h"
#include "Textbox.h"
#include "Direct2DWrite.h"
#include "CellProperty.h"
#include "UIElement.h"
#include "ResourceIDFactory.h"
#include "MyFile.h"
#include "MouseStateMachine.h"

LRESULT CTextboxWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Create direct
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);
	// Create textbox
	m_pControl = std::make_shared<CTextEditor>(
		this, m_spProp,
		[this](const std::wstring& text) 
		{
//			m_isSaved.notify_set(false);
		},
		[](const std::wstring& text) {});
	// Create mouse statemachine
	m_pMouseMachine = std::make_unique<CMouseStateMachine>(m_pControl.get());


	SetFocus();
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);
	CRect rcClient = GetClientRect();

	m_pDirect->BeginDraw();

	m_pDirect->GetHwndRenderTarget()->Clear(d2dw::CColorF(1.f, 1.f, 1.f));

	m_pControl->OnPaint(PaintEvent(this, &bHandled));

	//Paint Focused Line
	if (GetIsFocused()) {
		d2dw::CRectF rcFocus(m_pDirect->Pixels2Dips(rcClient));
		rcFocus.DeflateRect(1.0f, 1.0f);
		m_pDirect->DrawSolidRectangle(*(m_spProp->FocusedLine), rcFocus);
	}

	m_pDirect->EndDraw();

	::EndPaint(m_hWnd, &ps);
	return 0;
}

LRESULT CTextboxWnd::OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT CTextboxWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
	m_pControl->OnRect(RectEvent(this, m_pDirect->Pixels2Dips(GetClientRect()), &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pControl->OnSetCursor(SetCursorEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}


LRESULT CTextboxWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pControl->OnSetFocus(SetFocusEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}


LRESULT CTextboxWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pControl->OnKillFocus(KillFocusEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pControl->OnContextMenu(ContextMenuEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

void CTextboxWnd::OnFinalMessage(HWND m_hWnd){}

CTextboxWnd::CTextboxWnd(std::shared_ptr<TextEditorProperty> spProp)
	:CWnd(), m_spProp(spProp)
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

	AddMsgHandler(WM_SETCURSOR, &CTextboxWnd::OnSetCursor, this);
	AddMsgHandler(WM_SETFOCUS, &CTextboxWnd::OnSetFocus, this);
	AddMsgHandler(WM_KILLFOCUS, &CTextboxWnd::OnKillFocus, this);
	AddMsgHandler(WM_CONTEXTMENU, &CTextboxWnd::OnContextMenu, this);


	//UserInput
	AddMsgHandler(WM_LBUTTONDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
		SetFocus();
		return UserInputMachine_Message<LButtonDownEvent>(msg, wParam, lParam, bHandled);
		});
	AddMsgHandler(WM_LBUTTONUP, &CTextboxWnd::UserInputMachine_Message<LButtonUpEvent>, this);
	AddMsgHandler(WM_LBUTTONDBLCLK, &CTextboxWnd::UserInputMachine_Message<LButtonDblClkEvent>, this);
	AddMsgHandler(RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED"), &CTextboxWnd::UserInputMachine_Message<LButtonDblClkTimeExceedEvent>, this);
	AddMsgHandler(WM_MOUSEMOVE, &CTextboxWnd::UserInputMachine_Message<MouseMoveEvent>, this);
	AddMsgHandler(WM_MOUSELEAVE, &CTextboxWnd::UserInputMachine_Message<MouseLeaveEvent>, this);
	AddMsgHandler(WM_MOUSEWHEEL, &CTextboxWnd::UserInputMachine_Message<MouseWheelEvent>, this);
	AddMsgHandler(WM_CANCELMODE, &CTextboxWnd::UserInputMachine_Message<CancelModeEvent>, this);
	AddMsgHandler(WM_CAPTURECHANGED, &CTextboxWnd::UserInputMachine_Message<CaptureChangedEvent>, this);
	AddMsgHandler(WM_KEYDOWN, &CTextboxWnd::UserInputMachine_Message<KeyDownEvent>, this);
	AddMsgHandler(WM_SYSKEYDOWN, &CTextboxWnd::UserInputMachine_Message<SysKeyDownEvent>, this);
	AddMsgHandler(WM_CHAR, &CTextboxWnd::UserInputMachine_Message<CharEvent>, this);
}

CTextboxWnd::~CTextboxWnd() = default;

bool CTextboxWnd::GetIsFocused()const
{
	auto hWndAct = ::GetActiveWindow();
	auto hWndFcs = ::GetFocus();
	auto hWndFore = ::GetForegroundWindow();

	return hWndFcs == m_hWnd ||
		(HWND)::GetWindowWord(hWndAct, GWL_HWNDPARENT) == m_hWnd ||
		(HWND)::GetWindowWord(hWndFcs, GWL_HWNDPARENT) == m_hWnd ||
		(HWND)::GetWindowWord(hWndFore, GWL_HWNDPARENT) == m_hWnd ||
		(HWND)::GetWindow(hWndAct, GW_OWNER) == m_hWnd ||
		(HWND)::GetWindow(hWndFcs, GW_OWNER) == m_hWnd ||
		(HWND)::GetWindow(hWndFore, GW_OWNER) == m_hWnd ||
		(HWND)::GetParent(hWndAct) == m_hWnd ||
		(HWND)::GetParent(hWndFcs) == m_hWnd ||
		(HWND)::GetParent(hWndFore) == m_hWnd;
}

void CTextboxWnd::Update()
{
	m_pControl->Update();
}



