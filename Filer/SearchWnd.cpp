#include "SearchWnd.h"
#include "ProgressBar.h"
#include "CheckableFileGrid.h"
#include "IDL.h"
#include "ThreadPool.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"

UINT CSearchWnd::WM_INCREMENTMAX = ::RegisterWindowMessage(L"CSearchWnd::WM_INCREMENTMAX");
UINT CSearchWnd::WM_INCREMENTVALUE = ::RegisterWindowMessage(L"CSearchWnd::WM_INCREMENTVALUE");
UINT CSearchWnd::WM_ADDITEM = ::RegisterWindowMessage(L"CSearchWnd::WM_ADDITEM");

CSearchWnd::CSearchWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp, const CIDL& srcIDL)
	:CWnd(), m_srcIDL(srcIDL),
	m_pFileGrid(std::make_unique<CCheckableFileGrid>(spFilerGridViewProp)),
	m_pProgressbar(std::make_unique<d2dw::CProgressBar>(this, std::make_shared<ProgressProperty>()))

{
	m_rca
		.lpszClassName(L"CSearchWnd")
		.style(CS_VREDRAW | CS_HREDRAW)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	m_cwa
		.lpszWindowName(L"Search")
		.lpszClassName(L"CSearchWnd")
		.dwStyle(dwStyle);

	AddMsgHandler(WM_CREATE, &CSearchWnd::OnCreate, this);
	AddMsgHandler(WM_SIZE, &CSearchWnd::OnSize, this);
	AddMsgHandler(WM_CLOSE, &CSearchWnd::OnClose, this);
	AddMsgHandler(WM_DESTROY, &CSearchWnd::OnDestroy, this);
	AddMsgHandler(WM_PAINT, &CSearchWnd::OnPaint, this);
	AddMsgHandler(WM_INCREMENTMAX, &CSearchWnd::OnIncrementMax, this);
	AddMsgHandler(WM_INCREMENTVALUE, &CSearchWnd::OnIncrementValue, this);
	AddMsgHandler(WM_ADDITEM, &CSearchWnd::OnAddItem, this);

	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"SearchButton"), &CSearchWnd::OnCommandSearch, this);
	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CancelButton"), &CSearchWnd::OnCommandCancel, this);
	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CloseButton"), &CSearchWnd::OnCommandClose, this);
}

CSearchWnd::~CSearchWnd() = default;

CRect CSearchWnd::GetEditRect(const CRect& rcClient)const
{
	return CRect(rcClient.left + 5, rcClient.top + 5, rcClient.right - 5, rcClient.top + 25);
}

CRect CSearchWnd::GetProgressBarRect(const CRect& rcClient)const
{
	return CRect(rcClient.left + 5, rcClient.top + 25 + 5, rcClient.right - 5, rcClient.top + 50);
}

CRect CSearchWnd::GetFileGridRect(const CRect& rcClient)const
{
	return CRect(rcClient.left + 5, rcClient.top + 50 + 5, rcClient.right - 5, rcClient.bottom - 30);
}

CRect CSearchWnd::GetSearchBtnRect(const CRect& rcClient)const
{
	return CRect(rcClient.right - 170, rcClient.bottom - 25, rcClient.right - 170 + 52, rcClient.bottom - 25 + 22);
}

CRect CSearchWnd::GetCancelBtnRect(const CRect& rcClient)const
{
	return CRect(rcClient.right - 115, rcClient.bottom - 25, rcClient.right - 115 + 52, rcClient.bottom - 25 + 22);
}

CRect CSearchWnd::GetCloseBtnRect(const CRect& rcClient)const
{
	return CRect(rcClient.right - 60, rcClient.bottom - 25, rcClient.right - 60 + 52, rcClient.bottom - 25 + 22);
}


LRESULT CSearchWnd::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Direct2DWrite
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);

	//Size
	CRect rcClient = GetClientRect();

	//Edit
	m_edit.CreateWindowExArgument()
		.lpszClassName(WC_EDIT)
		.lpszWindowName(L"")
		.dwStyle(WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | ES_LEFT)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Edit"));

	m_edit.RegisterClassExArgument()
		.lpszClassName(WC_EDIT);

	m_edit.Create(m_hWnd, GetEditRect(rcClient));
	m_font = m_pFileGrid->GetCellProperty()->Format->Font.GetGDIFont();
	m_edit.SetFont(m_font);
	m_edit.EnableWindow(TRUE);

	//Progress
	m_pProgressbar->SetRect(m_pDirect->Pixels2Dips(GetProgressBarRect(rcClient)));

	//FileGrid
	m_pFileGrid->CreateWindowExArgument()
		.dwStyle(WS_CHILD | WS_VISIBLE | m_pFileGrid->CreateWindowExArgument().dwStyle());

	m_pFileGrid->Create(m_hWnd, GetFileGridRect(rcClient));

	//Search button
	m_buttonSearch.CreateWindowExArgument()
		.lpszClassName(WC_BUTTON)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"SearchButton"))
		.lpszWindowName(L"Search")
		.dwStyle(WS_CHILD | WS_VISIBLE | BP_PUSHBUTTON);

	m_buttonSearch.RegisterClassExArgument().lpszClassName(WC_BUTTON);
	m_buttonSearch.Create(m_hWnd, GetSearchBtnRect(rcClient));
	m_buttonSearch.EnableWindow(TRUE);

	//Close button
	m_buttonCancel.CreateWindowExArgument()
		.lpszClassName(WC_BUTTON)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CancelButton"))
		.lpszWindowName(L"Cancel")
		.dwStyle(WS_CHILD | WS_VISIBLE | BP_PUSHBUTTON);

	m_buttonCancel.RegisterClassExArgument().lpszClassName(WC_BUTTON);
	m_buttonCancel.Create(m_hWnd, GetCancelBtnRect(rcClient));
	m_buttonCancel.EnableWindow(TRUE);


	//Close button
	m_buttonClose.CreateWindowExArgument()
		.lpszClassName(WC_BUTTON)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CloseButton"))
		.lpszWindowName(L"Close")
		.dwStyle(WS_CHILD | WS_VISIBLE | BP_PUSHBUTTON);

	m_buttonClose.RegisterClassExArgument().lpszClassName(WC_BUTTON);
	m_buttonClose.Create(m_hWnd, GetCloseBtnRect(rcClient));
	m_buttonClose.EnableWindow(TRUE);

	return 0;
}




LRESULT CSearchWnd::OnCommandSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_buttonSearch.EnableWindow(FALSE);
	m_buttonCancel.EnableWindow(TRUE);

	std::wstring search = m_edit.GetWindowText();
	CThreadPool::GetInstance()->enqueue([search, this]()->void {

		std::function<void()> readMax = [this]()->void {
			PostMessage(WM_INCREMENTMAX, NULL, NULL);
		};
		
		std::function<void()> readValue = [this]()->void {
			PostMessage(WM_INCREMENTVALUE, NULL, NULL);
		};
		
		std::function<void(const CIDL&)> find = [this](const CIDL& findIDL)->void {
			//This should be Send Message to syncro
			m_newIDL = findIDL;
			SendMessage(WM_ADDITEM, NULL, NULL);
		};

		GetProgressBarPtr()->SetMin(0);
		GetProgressBarPtr()->SetMax(0);
		GetProgressBarPtr()->SetValue(0);

		auto fileCounter = CThreadPool::GetInstance()->enqueue([srcIDL = m_srcIDL, readMax]()->void {
			shell::CountFileInFolder(srcIDL, readMax);
		});

		auto searcher = CThreadPool::GetInstance()->enqueue([search, srcIDL = m_srcIDL, readValue, find]()->void {
			shell::SearchFileInFolder(search, srcIDL, readValue, find);
		});

		fileCounter.get();
		searcher.get();

		m_buttonSearch.EnableWindow(TRUE);
		m_buttonCancel.EnableWindow(FALSE);
		m_buttonClose.EnableWindow(TRUE);
	});

	return 0;
}

LRESULT CSearchWnd::OnCommandCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}


LRESULT CSearchWnd::OnCommandClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SendMessage(WM_CLOSE, NULL, NULL);
	return 0;
}

LRESULT CSearchWnd::OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Foreground Owner window
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}

	//Destroy
	m_pFileGrid->DestroyWindow();
	DestroyWindow();
	return 0;
}

LRESULT CSearchWnd::OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CSearchWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	m_pDirect->BeginDraw();

	m_pDirect->ClearSolid(BackgroundFill);
	m_pProgressbar->OnPaint(PaintEvent(this));

	m_pDirect->EndDraw();
	return 0;
}

LRESULT CSearchWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rcClient = GetClientRect();
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ (UINT)rcClient.Width(), (UINT)rcClient.Height() });

	m_edit.MoveWindow(GetEditRect(rcClient), TRUE);
	m_pProgressbar->SetRect(m_pDirect->Pixels2Dips(GetProgressBarRect(rcClient)));
	m_pFileGrid->MoveWindow(GetFileGridRect(rcClient), TRUE);
	m_buttonSearch.MoveWindow(GetSearchBtnRect(rcClient), TRUE);
	m_buttonCancel.MoveWindow(GetCancelBtnRect(rcClient), TRUE);
	m_buttonClose.MoveWindow(GetCloseBtnRect(rcClient), TRUE);

	return 0;
}

LRESULT CSearchWnd::OnAddItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pFileGrid->AddItem(CShellFileFactory::GetInstance()->CreateShellFilePtr(
		shell::DesktopBindToShellFolder(m_newIDL.CloneParentIDL()),
		m_newIDL.CloneParentIDL(),
		m_newIDL.CloneLastID()));
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CSearchWnd::OnIncrementMax(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pProgressbar->IncrementMax();
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CSearchWnd::OnIncrementValue(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pProgressbar->IncrementValue();
	InvalidateRect(NULL, FALSE);
	return 0;
}
