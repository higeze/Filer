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
#include "DropTargetManager.h"

LRESULT CALLBACK CD2DWWindow::StaticHostWndSubProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	HWND hWndOwner;
	HWND hWndTemp = hWnd;
	while (1){
		HWND hWndParent = ::GetParent(hWndTemp);
		if (hWndParent) {
			hWndTemp = hWndParent;
			continue;
		} else {
			hWndOwner = hWndTemp;
			break;
		}
	}
	CD2DWWindow *pWnd=(CD2DWWindow*)::GetWindowLongPtr(hWndOwner, GWLP_USERDATA);
	return pWnd->HostWndSubProc(hWnd, uMsg, wParam, lParam, uIdSubclass, dwRefData);
}

LRESULT CD2DWWindow::HostWndSubProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg) {
		case WM_ACTIVATE:
			if (!LOWORD(wParam)) {
				break;
			}
		case WM_SHOWWINDOW:
			if (!wParam) {
				break;
			}
		case WM_SETFOCUS:
		{
			//auto spControl = FindHostWndControlPtr(hWnd);
			//if (spControl && spControl->GetHostHWnd() == hWnd) {
			//	SetFocusedControlPtr(spControl);
			//}
			CD2DWControl* pControl = reinterpret_cast<CD2DWControl*>(dwRefData);
			SetFocusedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(pControl->shared_from_this()));
		}
		break;
	};
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


std::shared_ptr<CD2DWHostWndControl> CD2DWWindow::FindHostWndControlPtr(HWND hWnd)
{
	for (auto iter = m_childControls.cbegin(); iter != m_childControls.cend(); ++iter) {
		if (auto p = std::dynamic_pointer_cast<CD2DWHostWndControl>(*iter); p && p->GetHostHWnd() == hWnd) {
			return p;
		}
	}
	return nullptr;
}

CD2DWWindow::CD2DWWindow()
	:CWnd(),CD2DWControl(nullptr),
	m_pDispatcher(std::make_unique<CDispatcher>(this)),
	m_pDropTargetManager(std::make_unique<CDropTargetManager>(this)),
	m_pCapturedControl(nullptr)
{
	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CD2DWWindow"))
		.style(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground(NULL);
	CreateWindowExArgument()
		.lpszClassName(_T("CD2DWWindow"))
		.lpszWindowName(_T("D2DWWindow"))
		.dwStyle(WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE)
		.dwExStyle(WS_EX_ACCEPTFILES)
		.hMenu(nullptr);

	//Add Message
	AddMsgHandler(WM_CREATE, &CD2DWWindow::OnCreate, this);
	AddMsgHandler(WM_CLOSE, &CD2DWWindow::OnClose, this);
	AddMsgHandler(WM_SIZE, &CD2DWWindow::OnSize, this);
	AddMsgHandler(WM_PAINT, &CD2DWWindow::OnPaint, this);
	AddMsgHandler(WM_DISPLAYCHANGE, &CD2DWWindow::OnPaint, this);
	AddMsgHandler(WM_SYSCHAR, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
	{
		if (wParam == VK_RETURN) {
			bHandled = TRUE;
		}
		return 1;
	});

	AddMsgHandler(WM_ERASEBKGND, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
	{
		bHandled = TRUE;
		return 1;
	});

	AddMsgHandler(WM_SETCURSOR, Normal_Message(&CD2DWWindow::OnSetCursor));
	AddMsgHandler(WM_SETFOCUS, Normal_Message(&CD2DWWindow::OnSetFocus));
	AddMsgHandler(WM_KILLFOCUS, Normal_Message(&CD2DWWindow::OnKillFocus));

	AddMsgHandler(::RegisterWindowMessage(L"PreviewWindowSetFocus"), [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
	{
		CD2DWControl* pControl = reinterpret_cast<CD2DWControl*>(wParam);
		while (auto pParent = pControl->GetParentControlPtr()) {
			pParent->SetFocusedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(pControl->shared_from_this()));
			pControl = pParent;
		};
		bHandled = TRUE;
		return 1;
	});


	//UserInput
	AddMsgHandler(WM_LBUTTONDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
	{
		SetFocus();
		return UserInputMachine_Message<LButtonDownEvent>(msg, wParam, lParam, bHandled);
	});
	AddMsgHandler(WM_LBUTTONUP, &CD2DWWindow::UserInputMachine_Message<LButtonUpEvent>, this);
	//AddMsgHandler(WM_LBUTTONDBLCLK, &CD2DWWindow::UserInputMachine_Message<LButtonDblClkEvent>, this);
	AddMsgHandler(WM_LBUTTONDBLCLK, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
	{
		return UserInputMachine_Message<LButtonDblClkEvent>(msg, wParam, lParam, bHandled);
	});

	AddMsgHandler(RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED"), &CD2DWWindow::UserInputMachine_Message<LButtonDblClkTimeExceedEvent>, this);
	AddMsgHandler(WM_RBUTTONDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
	{
		SetFocus();
		return UserInputMachine_Message<RButtonDownEvent>(msg, wParam, lParam, bHandled);
	});
	AddMsgHandler(WM_CONTEXTMENU, &CD2DWWindow::UserInputMachine_Message<ContextMenuEvent>, this);

	AddMsgHandler(WM_MOUSEMOVE, &CD2DWWindow::UserInputMachine_Message<MouseMoveEvent>, this);
	AddMsgHandler(WM_MOUSELEAVE, &CD2DWWindow::UserInputMachine_Message<MouseLeaveEvent>, this);
	AddMsgHandler(WM_MOUSEWHEEL, &CD2DWWindow::UserInputMachine_Message<MouseWheelEvent>, this);
	AddMsgHandler(WM_CANCELMODE, &CD2DWWindow::UserInputMachine_Message<CancelModeEvent>, this);
	AddMsgHandler(WM_CAPTURECHANGED, &CD2DWWindow::UserInputMachine_Message<CaptureChangedEvent>, this);
	AddMsgHandler(WM_KEYDOWN, &CD2DWWindow::UserInputMachine_Message<KeyDownEvent>, this);
	AddMsgHandler(WM_KEYUP, &CD2DWWindow::UserInputMachine_Message<KeyUpEvent>, this);
	AddMsgHandler(WM_SYSKEYDOWN, &CD2DWWindow::UserInputMachine_Message<SysKeyDownEvent>, this);
	AddMsgHandler(WM_CHAR, &CD2DWWindow::UserInputMachine_Message<CharEvent>, this);
	AddMsgHandler(CDispatcher::WM_DISPATCHER, &CDispatcher::OnDispatcher, m_pDispatcher.get());
}

CD2DWWindow::~CD2DWWindow() = default;

CPointF CD2DWWindow::GetCursorPosInWnd() const
{ 
	CPoint pt;
	::GetCursorPos(&pt);
	::ScreenToClient(m_hWnd, &pt);
	return GetDirectPtr()->Pixels2Dips(pt);
}

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

LRESULT CD2DWWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetFocus();
	m_pDirect = std::make_shared<CDirect2DWrite>(m_hWnd);
	m_pMouseMachine = std::make_unique<CMouseStateMachine>(this);
	::RegisterDragDrop(m_hWnd, m_pDropTargetManager.get());
	OnCreate(CreateEvt(this, this, wParam, lParam, &bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CD2DWWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}
	auto e = ClosingEvent(this, wParam, lParam, &bHandled);
	OnClosing(e);
	if (!(*e.CancelPtr)) {
		OnClose(CloseEvent(this, wParam, lParam, &bHandled));
		::RevokeDragDrop(m_hWnd);
		DestroyWindow();
	}
	return 0;
}

LRESULT CD2DWWindow::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	CD2DWControl::OnCommand(CommandEvent(this, wParam, lParam, &bHandled));
	return 0;
}


LRESULT CD2DWWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_pDirect->GetD2DDeviceContext()->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
	m_pDirect->Resize();
	OnRect(RectEvent(this, GetDirectPtr()->Pixels2Dips(GetClientRect())));
	return 0;
}

LRESULT CD2DWWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	m_pDirect->BeginDraw(dc);

	m_pDirect->ClearSolid(CColorF(1.f, 1.f, 1.f));

	OnPaint(PaintEvent(this, &bHandled));

	m_pDirect->EndDraw();
	return 0;
}

void CD2DWWindow::OnKeyDown(const KeyDownEvent& e)
{
	*(e.HandledPtr) = FALSE;
	SendFocused(&CD2DWControl::OnKeyDown, e);

	if (*(e.HandledPtr) == FALSE) {
		switch (e.Char){
			case VK_TAB:
				{
					auto shift = ::GetAsyncKeyState(VK_SHIFT);
					auto focused_iter = std::find(m_childControls.cbegin(), m_childControls.cend(), GetFocusedControlPtr());
					if (focused_iter == m_childControls.cend()) {
						break;
					}

					for (auto iter = std::next(focused_iter, 1) ; iter != m_childControls.cend(); ++iter) {
						if ((*iter)->GetIsTabStop()) {
							*(e.HandledPtr) = TRUE;
							SetFocusedControlPtr((*iter));
							break;
						}
					}
					for (auto iter = m_childControls.cbegin(); iter != focused_iter; ++iter) {
						if ((*iter)->GetIsTabStop()) {
							*(e.HandledPtr) = TRUE;
							SetFocusedControlPtr((*iter));
							break;
						}
					}
				}
				break;
			default:
				break;
		}
	}
}




