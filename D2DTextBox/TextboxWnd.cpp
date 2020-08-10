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
	m_pTxtbox = std::make_unique<D2DTextbox2>(
		this, m_spProp,
		m_text,
		[this](const std::wstring& text) 
		{
			m_text = text;
			m_isSaved.notify_set(false);
		},
		[](const std::wstring& text) {});
	// Create mouse statemachine
	m_pMouseMachine = std::make_unique<CMouseStateMachine>(m_pTxtbox.get());


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

	m_pTxtbox->OnPaint(PaintEvent(this));

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
	m_pTxtbox->OnRect(RectEvent(this, m_pDirect->Pixels2Dips(GetClientRect()), bHandled)
	);
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
	if ((wParam == 'O') && ::GetAsyncKeyState(VK_CONTROL)) {
		Open();
	} else if ((wParam == 'S') && ::GetAsyncKeyState(VK_CONTROL)) {
		if (m_path.get().empty()) {
			Save();
		} else {
			Save(m_path);
		}
	} else {
		m_pTxtbox->OnKeyDown(KeyDownEvent(this, wParam, lParam, bHandled));
	}
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnSetCursor(SetCursorEvent(this, wParam, lParam, bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}


LRESULT CTextboxWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnSetFocus(SetFocusEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}


LRESULT CTextboxWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnKillFocus(KillFocusEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CTextboxWnd::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnMouseWheel(MouseWheelEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	return 0;
}


LRESULT CTextboxWnd::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pTxtbox->OnContextMenu(ContextMenuEvent(this, wParam, lParam, bHandled));
	InvalidateRect(NULL, FALSE);
	return 0;
}

void CTextboxWnd::OnFinalMessage(HWND m_hWnd){}


CTextboxWnd::CTextboxWnd(std::shared_ptr<TextboxProperty> spProp)
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

	AddMsgHandler(WM_KEYDOWN, &CTextboxWnd::OnKeyDown, this);
	AddMsgHandler(WM_SYSKEYDOWN, &CTextboxWnd::OnKeyDown, this);
	AddMsgHandler(WM_CHAR, &CTextboxWnd::OnChar, this);

	AddMsgHandler(WM_SETCURSOR, &CTextboxWnd::OnSetCursor, this);
	AddMsgHandler(WM_SETFOCUS, &CTextboxWnd::OnSetFocus, this);
	AddMsgHandler(WM_KILLFOCUS, &CTextboxWnd::OnKillFocus, this);
	AddMsgHandler(WM_MOUSEWHEEL, &CTextboxWnd::OnMouseWheel, this);
	AddMsgHandler(WM_CONTEXTMENU, &CTextboxWnd::OnContextMenu, this);

	//Mouse
	AddMsgHandler(WM_LBUTTONDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(LButtonDownEvent(this, wParam, lParam));
		SetFocus();
		InvalidateRect(NULL, FALSE);
		return 0;
		});
	AddMsgHandler(WM_LBUTTONUP, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(LButtonUpEvent(this, wParam, lParam));
		InvalidateRect(NULL, FALSE);
		return 0;
		});
	AddMsgHandler(WM_LBUTTONDBLCLK, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(LButtonDblClkEvent(this, wParam, lParam));
		InvalidateRect(NULL, FALSE);
		return 0;
		});
	AddMsgHandler(RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED"), [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(LButtonDblClkTimeExceedEvent(this, wParam, lParam));
		InvalidateRect(NULL, FALSE);
		return 0;
		});
	AddMsgHandler(WM_MOUSEMOVE, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(MouseMoveEvent(this, wParam, lParam));
		InvalidateRect(NULL, FALSE);
		return 0;
		});
	AddMsgHandler(WM_MOUSELEAVE, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(MouseLeaveEvent(this, wParam, lParam));
		InvalidateRect(NULL, FALSE);
		return 0;
		});
	AddMsgHandler(WM_CANCELMODE, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(CancelModeEvent(this, wParam, lParam));
		InvalidateRect(NULL, FALSE);
		return 0;
		});
	AddMsgHandler(WM_CAPTURECHANGED, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT {
		m_pMouseMachine->process_event(CaptureChangedEvent(this, wParam, lParam));
		InvalidateRect(NULL, FALSE);
		return 0;
		});



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


void CTextboxWnd::Open()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//ofn.lpstrDefExt = L"txt";

	if (!GetOpenFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			//wsprintf(szErrMsg, L"Error code : %d", errCode);
			//MessageBox(NULL, szErrMsg, L"GetOpenFileName", MB_OK);
		}
	} else {
		::ReleaseBuffer(path);
		Open(path);
	}
}

void CTextboxWnd::Open(const std::wstring& path)
{
	m_pTxtbox->Clear();
	if (::PathFileExists(path.c_str())) {
		m_path.notify_set(path);
		m_pTxtbox->GetText().notify_assign(str2wstr(CFile::ReadAllString<char>(path)));
		m_isSaved.notify_set(true);
	} else {
		m_path.notify_set(L"");
		m_isSaved.notify_set(false);
	}

}

void CTextboxWnd::Save()
{
	std::wstring path;
	if (m_path.get().empty()) {
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = m_hWnd;
		//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
		ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = L"Save as";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
		//ofn.lpstrDefExt = L"txt";

		if (!GetSaveFileName(&ofn)) {
			DWORD errCode = CommDlgExtendedError();
			if (errCode) {
				//wsprintf(szErrMsg, L"Error code : %d", errCode);
				//MessageBox(NULL, szErrMsg, L"GetOpenFileName", MB_OK);
			}
		} else {
			::ReleaseBuffer(path);
		}
	}
	//Serialize
	try {
		Save(path);
	}
	catch (/*_com_error &e*/...) {
	}

}

void CTextboxWnd::Save(const std::wstring& path)
{
	m_path.notify_set(path);
	m_isSaved.notify_set(true);
	CFile::WriteAllString(path, wstr2str(m_text));
}

void CTextboxWnd::Update()
{
	InvalidateRect(NULL, FALSE);
	m_pTxtbox->Update();
}

