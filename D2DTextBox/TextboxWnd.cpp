#include "text_stdafx.h"
#include <Shellapi.h>
#include "TextboxWnd.h"
#include "Textbox.h"
#include "Direct2DWrite.h"
#include "CellProperty.h"
#include "UIElement.h"
#include "ResourceIDFactory.h"
#include "MyFile.h"

LRESULT CTextboxWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Create direct
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);
	// Cteate textbox
	m_pTxtbox = std::make_unique<D2DTextbox2>(this, m_spProp,
											 [this]() {return m_text; },
											 [this](const std::wstring& text) {m_text = text; },
		[this](const std::wstring& text) {m_text = text; },
											 [](const std::wstring& text) {});

	SetFocus();
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

LRESULT CTextboxWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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
	SetFocus();
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

void CTextboxWnd::OnFinalMessage(HWND m_hWnd){}


CTextboxWnd::CTextboxWnd(std::shared_ptr<TextboxProperty> pProp)
	:CWnd(), m_spProp(pProp)
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
	if (::PathFileExists(path.c_str())) {
		m_path.notify_set(path);
		m_pTxtbox->GetText().notify_assign(str2wstr(CFile::ReadAllString<char>(path)));
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
	CFile::WriteAllString(path, wstr2str(m_text));
}

