#include "text_stdafx.h"
#include <Shellapi.h>
#include "D2DWWindow.h"
#include "Textbox.h"
#include "Direct2DWrite.h"
#include "CellProperty.h"
#include "UIElement.h"
#include "ResourceIDFactory.h"
#include "MyFile.h"
#include "MouseStateMachine.h"
#include "Dispatcher.h"

void CD2DWWindow::SetFocusControl(const std::shared_ptr<CD2DWControl>& spControl)
{
	if (spControl->GetIsFocusable()) {
		if (m_pFocusedControl) {
			m_pFocusedControl->OnKillFocus(KillFocusEvent(this, 0, 0, nullptr));
		}
		m_pFocusedControl = spControl;
		m_pFocusedControl->OnSetFocus(SetFocusEvent(this, 0, 0, nullptr));
	}
}

LRESULT CD2DWWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}
	OnClose(CloseEvent(this, wParam, lParam, &bHandled));
	DestroyWindow();
	return 0;
}

LRESULT CD2DWWindow::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (GetFocusedControlPtr())GetFocusedControlPtr()->OnSetCursor(SetCursorEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CD2DWWindow::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (GetFocusedControlPtr())GetFocusedControlPtr()->OnSetFocus(SetFocusEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CD2DWWindow::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (GetFocusedControlPtr())GetFocusedControlPtr()->OnKillFocus(KillFocusEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CD2DWWindow::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OnCommand(CommandEvent(this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

void CD2DWWindow::OnFinalMessage(HWND m_hWnd) {}

CD2DWWindow::CD2DWWindow()
	:CWnd(),CD2DWControl(nullptr),m_pDispatcher(std::make_unique<CDispatcher>(this))
{
	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CD2DWWindow"))
		.style(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground(NULL);
	CreateWindowExArgument()
		.lpszClassName(_T("CD2DWWIndow"))
		.lpszWindowName(_T("D2DWWindow"))
		.dwStyle(WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE)
		.dwExStyle(WS_EX_ACCEPTFILES)
		.hMenu(nullptr);

	//Add Message
	AddMsgHandler(WM_CREATE, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
		{
			SetFocus();
			m_pDirect = std::make_shared<CDirect2DWrite>(m_hWnd);
			m_pMouseMachine = std::make_unique<CMouseStateMachine>(this);
			OnCreate(CreateEvt(this, wParam, lParam, &bHandled));
			InvalidateRect(NULL, FALSE);
			return 0;
		});
	AddMsgHandler(WM_CLOSE, &CD2DWWindow::OnClose, this);

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

		m_pDirect->ClearSolid(CColorF(1.f, 1.f, 1.f));

		OnPaint(PaintEvent(this, &bHandled));

		m_pDirect->EndDraw();
		return 0;
	};
	AddMsgHandler(WM_PAINT, onPaint);
	AddMsgHandler(WM_DISPLAYCHANGE, onPaint);

	AddMsgHandler(WM_SETCURSOR, &CD2DWWindow::OnSetCursor, this);
	AddMsgHandler(WM_SETFOCUS, &CD2DWWindow::OnSetFocus, this);
	AddMsgHandler(WM_KILLFOCUS, &CD2DWWindow::OnKillFocus, this);


	//UserInput
	AddMsgHandler(WM_LBUTTONDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
		{
			SetFocus();
			return UserInputMachine_Message<LButtonDownEvent>(msg, wParam, lParam, bHandled);
		});
	AddMsgHandler(WM_LBUTTONUP, &CD2DWWindow::UserInputMachine_Message<LButtonUpEvent>, this);
	AddMsgHandler(WM_LBUTTONDBLCLK, &CD2DWWindow::UserInputMachine_Message<LButtonDblClkEvent>, this);
	AddMsgHandler(RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED"), &CD2DWWindow::UserInputMachine_Message<LButtonDblClkTimeExceedEvent>, this);
	AddMsgHandler(WM_RBUTTONDOWN, &CD2DWWindow::UserInputMachine_Message<RButtonDownEvent>, this);
	AddMsgHandler(WM_CONTEXTMENU, &CD2DWWindow::UserInputMachine_Message<ContextMenuEvent>, this);

	AddMsgHandler(WM_MOUSEMOVE, &CD2DWWindow::UserInputMachine_Message<MouseMoveEvent>, this);
	AddMsgHandler(WM_MOUSELEAVE, &CD2DWWindow::UserInputMachine_Message<MouseLeaveEvent>, this);
	AddMsgHandler(WM_MOUSEWHEEL, &CD2DWWindow::UserInputMachine_Message<MouseWheelEvent>, this);
	AddMsgHandler(WM_CANCELMODE, &CD2DWWindow::UserInputMachine_Message<CancelModeEvent>, this);
	AddMsgHandler(WM_CAPTURECHANGED, &CD2DWWindow::UserInputMachine_Message<CaptureChangedEvent>, this);
	AddMsgHandler(WM_KEYDOWN, &CD2DWWindow::UserInputMachine_Message<KeyDownEvent>, this);
	AddMsgHandler(WM_SYSKEYDOWN, &CD2DWWindow::UserInputMachine_Message<SysKeyDownEvent>, this);
	AddMsgHandler(WM_CHAR, &CD2DWWindow::UserInputMachine_Message<CharEvent>, this);
	AddMsgHandler(CDispatcher::WM_DISPATCHER, &CDispatcher::OnDispatcher, m_pDispatcher.get());
}

CD2DWWindow::~CD2DWWindow() = default;

bool CD2DWWindow::GetIsFocused()const
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

void CD2DWWindow::Update()
{
	GetFocusedControlPtr()->Update();
}



