#include "PreviewControl.h"
#include "PreviewControlProperty.h"
#include "PreviewWnd.h"
#include "Debug.h"

/**********/
/* Static */
/**********/

/**************************/
/* Constructor/Destructor */
/**************************/

CPreviewControl::CPreviewControl(CD2DWControl* pParentControl, const std::shared_ptr<PreviewControlProperty>& pProp)
	:CD2DWHostWndControl(pParentControl),
	m_pWnd(std::make_unique<CPreviewWnd>()),
    m_pProp(pProp){}

CPreviewControl::~CPreviewControl() = default;

void CPreviewControl::Open(const std::wstring& path)
{
	m_pWnd->ShowWindow(SW_HIDE);
	m_pWnd->Open(path);
	m_pWnd->ShowWindow(SW_SHOW);

	//HWND hWndPreview = GetWindow(m_pWnd->m_hWnd, GW_CHILD);
	//m_pHooker.reset();
	//m_pHooker = std::make_unique<CPreviewWindowsHooker>(m_pWnd->m_hWnd, this, hWndPreview);
}

void CPreviewControl::Open()
{
	m_pWnd->Open();
}

void CPreviewControl::Close()
{
	//m_pHooker.reset();
	m_pWnd->ShowWindow(SW_HIDE);
}

std::tuple<CRect> CPreviewControl::GetRects() const
{
	CRectF rcClient(GetRectInWnd());
	FLOAT lineWidth = m_pProp->FocusedLine.Width + 0.5f;
	rcClient.DeflateRect(lineWidth);

	return { CRect(
		static_cast<LONG>(std::ceil(rcClient.left)),
		static_cast<LONG>(std::ceil(rcClient.top)),
		static_cast<LONG>(std::floor(rcClient.right)),
		static_cast<LONG>(std::floor(rcClient.bottom))
	) };
}

/****************/
/* EventHandler */
/****************/

void CPreviewControl::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rc] = GetRects();
	m_pWnd->CreateWindowExArgument().dwStyle(m_pWnd->CreateWindowExArgument().dwStyle() | WS_CHILD);
	m_pWnd->Create(GetWndPtr()->m_hWnd, rc);
	HWND hWnd = m_pWnd->m_hWnd;
	while (HWND hWndParent = ::GetParent(hWnd)){
		hWnd = hWndParent;
	}
	m_pWnd->AddMsgHandler(WM_PARENTNOTIFY, [this, hWnd](UINT, WPARAM, LPARAM, BOOL& bHandled)->LRESULT {
		::PostMessage(hWnd, ::RegisterWindowMessage(L"PreviewWindowSetFocus"), (WPARAM)this, (LPARAM)NULL);
		return 0;
	});
	m_pWnd->ShowWindow(SW_HIDE);
}

void CPreviewControl::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rc] = GetRects();
	m_pWnd->MoveWindow(rc, TRUE);
}

void CPreviewControl::OnPaint(const PaintEvent& e)
{
	//Paint Background
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_pProp->NormalFill, GetRectInWnd());
	//Paint Focused Line
	CRectF rcBorder(GetRectInWnd());
	rcBorder.DeflateRect(1.0f, 1.0f);
	if (GetIsFocused()) {
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(m_pProp->FocusedLine, rcBorder);
	}
}

void CPreviewControl::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CPreviewControl::OnDestroy(const DestroyEvent& e)
{
	CD2DWControl::OnDestroy(e);
	//m_pHooker.reset();
	m_pWnd->DestroyWindow();
}

void CPreviewControl::OnSetFocus(const SetFocusEvent& e)
{
	CD2DWControl::OnSetFocus(e);
	m_pWnd->SetFocus();
}

void CPreviewControl::OnEnable(const EnableEvent& e)
{
	if (e.Enable) {
		m_pWnd->ShowWindow(SW_SHOW);
	} else {
		Close();
	}
}

void CPreviewControl::OnWndKillFocus(const KillFocusEvent& e)
{
}

void CPreviewControl::OnKeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (e.Char) {
		case 'O':
			if (ctrl) {
				Open();
			}
			break;
		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}