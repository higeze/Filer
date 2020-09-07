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

LRESULT CTextboxWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}

	DestroyWindow();
	return 0;
}

LRESULT CTextboxWnd::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (GetFocusedControlPtr())GetFocusedControlPtr()->OnSetCursor(SetCursorEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (GetFocusedControlPtr())GetFocusedControlPtr()->OnSetFocus(SetFocusEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (GetFocusedControlPtr())GetFocusedControlPtr()->OnKillFocus(KillFocusEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	GetFocusedControlPtr()->OnCommand(CommandEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

void CTextboxWnd::OnFinalMessage(HWND m_hWnd) {}

CTextboxWnd::CTextboxWnd()
	:CWnd()
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
		.dwExStyle(WS_EX_ACCEPTFILES)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"InplaceEditWnd"));

	//Add Message
	AddMsgHandler(WM_CREATE, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
		{
			m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);
			m_pMouseMachine = std::make_unique<CMouseStateMachine>(this);
			OnCreate(CreateEvent(this, wParam, lParam, &bHandled));
			SetFocus();
			SendAll(&CUIElement::OnCreate, CreateEvent(this, wParam, lParam, &bHandled));
			InvalidateRect(NULL, FALSE);
			return 0;
		});
	AddMsgHandler(WM_CLOSE, &CTextboxWnd::OnClose, this);

	AddMsgHandler(WM_ERASEBKGND, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
		{
			bHandled = TRUE;
			return 1;
		});
	AddMsgHandler(WM_SIZE, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
		{
			m_pDirect->GetHwndRenderTarget()->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
			OnRect(RectEvent(this, GetDirectPtr()->Pixels2Dips(GetClientRect())));
			return 0;
		});

	auto onPaint = [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
		{
			CPaintDC dc(m_hWnd);
			m_pDirect->BeginDraw();

			m_pDirect->ClearSolid(d2dw::CColorF(1.f, 1.f, 1.f));

			OnPaint(PaintEvent(this, &bHandled));

			m_pDirect->EndDraw();
			return 0;
		};
	AddMsgHandler(WM_PAINT, onPaint);
	AddMsgHandler(WM_DISPLAYCHANGE, onPaint);

	AddMsgHandler(WM_SETCURSOR, &CTextboxWnd::OnSetCursor, this);
	AddMsgHandler(WM_SETFOCUS, &CTextboxWnd::OnSetFocus, this);
	AddMsgHandler(WM_KILLFOCUS, &CTextboxWnd::OnKillFocus, this);


	//UserInput
	AddMsgHandler(WM_LBUTTONDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
		{
			SetFocus();
			return UserInputMachine_Message<LButtonDownEvent>(msg, wParam, lParam, bHandled);
		});
	AddMsgHandler(WM_LBUTTONUP, &CTextboxWnd::UserInputMachine_Message<LButtonUpEvent>, this);
	AddMsgHandler(WM_LBUTTONDBLCLK, &CTextboxWnd::UserInputMachine_Message<LButtonDblClkEvent>, this);
	AddMsgHandler(RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED"), &CTextboxWnd::UserInputMachine_Message<LButtonDblClkTimeExceedEvent>, this);
	AddMsgHandler(WM_RBUTTONDOWN, &CTextboxWnd::UserInputMachine_Message<RButtonDownEvent>, this);
	AddMsgHandler(WM_CONTEXTMENU, &CTextboxWnd::UserInputMachine_Message<ContextMenuEvent>, this);

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
	GetFocusedControlPtr()->Update();
}



