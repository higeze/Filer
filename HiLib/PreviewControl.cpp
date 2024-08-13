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

CPreviewControl::CPreviewControl(CD2DWControl* pParentControl)
	:CD2DWHostWndControl(pParentControl),
	Dummy(std::make_shared<int>(0)),
	Doc(),
	m_pWnd(std::make_unique<CPreviewWnd>())
{
	Doc.subscribe([this](auto doc) {
		Open(doc.GetPath());
	}, Dummy);
}

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
	FLOAT lineWidth = GetFocusedBorder().Width + 0.5f;
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

void CPreviewControl::Arrange(const CRectF& e)
{
	CD2DWControl::Arrange(e);
	auto [rc] = GetRects();
	m_pWnd->MoveWindow(rc, TRUE);
}

void CPreviewControl::OnPaint(const PaintEvent& e)
{
	//Paint Background
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());
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

void CPreviewControl::OnKillFocus(const KillFocusEvent& e)
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
