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
#include "TSFManager.h"

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
			GetWndPtr()->SetFocusToControl(std::dynamic_pointer_cast<CD2DWControl>(pControl->shared_from_this()));
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
			GetWndPtr()->SetFocusToControl(std::dynamic_pointer_cast<CD2DWControl>(pControl->shared_from_this()));
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

	AddMsgHandler(WM_SYSKEYDOWN, &CD2DWWindow::UserInputMachine_Message<SysKeyDownEvent>, this);
	AddMsgHandler(WM_CHAR, &CD2DWWindow::UserInputMachine_Message<CharEvent>, this);
	AddMsgHandler(CTSFManager::WM_TSF_STARTCOMPOSITION, &CD2DWWindow::UserInputMachine_Message<ImeStartCompositionEvent>, this);
	AddMsgHandler(CDispatcher::WM_DISPATCHER, &CDispatcher::OnDispatcher, m_pDispatcher.get());
	AddMsgHandler(WM_KEYDOWN, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
		//BOOL fEaten;
		//if (!(CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->TestKeyDown(wParam, lParam, &fEaten) == S_OK && fEaten &&
		//	CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->KeyDown(wParam, lParam, &fEaten) == S_OK && fEaten)) {
			m_pMouseMachine->process_event(KeyDownEvent(this, wParam, lParam, &bHandled));
			InvalidateRect(NULL, FALSE);
		//}
		return 0;
	});
	AddMsgHandler(WM_KEYUP, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {	
		//BOOL fEaten;
		//if (!(CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->TestKeyUp(wParam, lParam, &fEaten) == S_OK && fEaten &&
		//	CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->KeyUp(wParam, lParam, &fEaten) == S_OK && fEaten)) {
			m_pMouseMachine->process_event(KeyUpEvent(this, wParam, lParam, &bHandled));
			InvalidateRect(NULL, FALSE);
		//}
		return 0;
	});
	AddMsgHandler(CKeyTraceEventSink::WM_KEYTRACEDOWN, &CD2DWWindow::UserInputMachine_Message<KeyTraceDownEvent>, this);
	AddMsgHandler(CKeyTraceEventSink::WM_KEYTRACEUP, &CD2DWWindow::UserInputMachine_Message<KeyTraceUpEvent>, this);

}

CD2DWWindow::~CD2DWWindow() = default;

CPointF CD2DWWindow::GetCursorPosInWnd() const
{ 
	CPoint pt;
	::GetCursorPos(&pt);
	::ScreenToClient(m_hWnd, &pt);
	return GetDirectPtr()->Pixels2Dips(pt);
}

bool CD2DWWindow::IsFocused()const
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


bool CD2DWWindow::GetIsFocused()const
{
	return IsFocused();
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
	m_pDirect->Resize();
	OnSize(SizeEvent(this, GetDirectPtr()->Pixels2Dips(GetClientRect().Size())));
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

void CD2DWWindow::OnSize(const SizeEvent& e)
{
	Measure(e.Size);
	Arrange(CRectF(e.Size));
}

void CD2DWWindow::OnKeyDown(const KeyDownEvent& e)
{
	BubbleFocusMessage(&CD2DWControl::OnKeyDown, e);

	if (!*e.HandledPtr) {
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
							GetWndPtr()->SetFocusToControl((*iter));
							break;
						}
					}
					for (auto iter = m_childControls.cbegin(); iter != focused_iter; ++iter) {
						if ((*iter)->GetIsTabStop()) {
							*(e.HandledPtr) = TRUE;
							GetWndPtr()->SetFocusToControl((*iter));
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

//In:Already Mouse In
//Entry:First entry
//Leave:First exit

void CD2DWWindow::ProcessMouseLeaveRecursive(const std::shared_ptr<CD2DWControl> pLeave, const MouseMoveEvent& e)
{
	if (pLeave) {
		if (pLeave->m_pMouseControl) {
			ProcessMouseLeaveRecursive(pLeave->m_pMouseControl, e);
			pLeave->m_pMouseControl = nullptr;
		}
		pLeave->OnMouseLeave(MouseLeaveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
	}
}
void CD2DWWindow::ProcessMouseEntryRecursive(const std::shared_ptr<CD2DWControl> pEntry, const MouseMoveEvent& e)
{
	if (pEntry) {
		auto iter = std::find_if(pEntry->m_childControls.crbegin(), pEntry->m_childControls.crend(),
			[&](const std::shared_ptr<CD2DWControl>& pChild) {
			return *pChild->IsEnabled && pChild->GetRectInWnd().PtInRect(e.PointInWnd);
		});

		if (iter == pEntry->m_childControls.crend()) {//Mouse is NOT on child control, but on me.
			ProcessMouseLeaveRecursive(pEntry->m_pMouseControl, e);
		} else if (pEntry->m_pMouseControl != *iter) {//Mouse is on child control and different with previous one
			ProcessMouseLeaveRecursive(pEntry->m_pMouseControl, e);
			pEntry->m_pMouseControl = *iter;
			ProcessMouseEntryRecursive(pEntry->m_pMouseControl, e);
		} else {//Mouse is on same child control

		}
		pEntry->OnMouseEnter(MouseEnterEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y), e.HandledPtr));
	}
}

void CD2DWWindow::ProcessMouseInRecursive(const std::shared_ptr<CD2DWControl> pEntry, const MouseMoveEvent& e)
{
	if (pEntry) {
		auto iter = std::find_if(pEntry->m_childControls.crbegin(), pEntry->m_childControls.crend(),
			[&](const std::shared_ptr<CD2DWControl>& pChild) {
			return *pChild->IsEnabled && pChild->GetRectInWnd().PtInRect(e.PointInWnd);
		});

		if (iter == pEntry->m_childControls.crend()) {//Mouse is NOT on child control, but on me.
			ProcessMouseLeaveRecursive(pEntry->m_pMouseControl, e);
		} else if (pEntry->m_pMouseControl != *iter) {//Mouse is on child control and different with previous one
			ProcessMouseLeaveRecursive(pEntry->m_pMouseControl, e);
			pEntry->m_pMouseControl = *iter;
			ProcessMouseEntryRecursive(pEntry->m_pMouseControl, e);
		} else {//Mouse is on same child control
			ProcessMouseInRecursive(pEntry->m_pMouseControl, e);
		}
	}
}

void CD2DWWindow::ProcessMouseEntryLeave(const MouseMoveEvent& e)
{
	if (GetRectInWnd().PtInRect(e.PointInWnd)) {
		ProcessMouseInRecursive(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()), e);
	} else {
		ProcessMouseLeaveRecursive(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()), e);
	}
}

void CD2DWWindow::OnPaint(const PaintEvent& e)
{ 
	ProcessMessageToAll(&CD2DWControl::OnPaint, e); 
	//Cur Focused
	std::vector<std::shared_ptr<CD2DWControl>> tunnelCurControls;
	std::shared_ptr<CD2DWControl> pParentControl = std::dynamic_pointer_cast<CD2DWControl>(shared_from_this());
	while (pParentControl->m_pFocusedControl) {
		tunnelCurControls.push_back(pParentControl->m_pFocusedControl);
		pParentControl = pParentControl->m_pFocusedControl;
	}

	SolidLine FocusedLine = SolidLine(22.f / 255.f, 160.f / 255.f, 133.f / 255.f, 1.0f, 1.0f);
	for (auto iter = tunnelCurControls.rbegin(); iter != tunnelCurControls.rend(); iter++) {
		CRectF rcFocus((*iter)->GetRectInWnd());
		rcFocus.DeflateRect(1.0f, 1.0f);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(FocusedLine, rcFocus);
	}

	if (m_pToolTip) {
		m_pToolTip->OnPaint(e);
	}

}


void CD2DWWindow::OnMouseMove(const MouseMoveEvent& e)
{
	ProcessMouseEntryLeave(e);
	BubbleMouseMessage(&CD2DWControl::OnMouseMove, e);
}

void CD2DWWindow::OnMouseLeave(const MouseLeaveEvent& e)
{
	if (m_pMouseControl) {
		ProcessMouseLeaveRecursive(m_pMouseControl, MouseMoveEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		m_pMouseControl = nullptr;
	}
}




