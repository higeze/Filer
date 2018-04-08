#include "FilerTabGridView.h"

CFilerTabGridView::CFilerTabGridView()
{
	CreateWindowExArgument()
		.dwStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | TCS_HOTTRACK | TCS_FLATBUTTONS | TCS_MULTILINE);

	AddMsgHandler(WM_CREATE, &CFilerTabGridView::OnCreate, this);
	AddMsgHandler(WM_SIZE, &CFilerTabGridView::OnSize, this);
	AddMsgHandler(WM_CLOSE, &CFilerTabGridView::OnClose, this);
	//AddMsgHandler(WM_DESTROY, &CFilerTabGridView::OnDestroy, this);
	AddMsgHandler(WM_KEYDOWN,&CFilerTabGridView::OnKeyDown,this);

	AddCmdIDHandler(IDM_NEWTAB, &CFilerTabGridView::OnCommandNewTab, this);
	AddCmdIDHandler(IDM_CLONETAB, &CFilerTabGridView::OnCommandCloneTab, this);
	AddCmdIDHandler(IDM_CLOSETAB, &CFilerTabGridView::OnCommandCloseTab, this);
	AddCmdIDHandler(IDM_CLOSEALLBUTTHISTAB, &CFilerTabGridView::OnCommandCloseAllButThisTab, this);

}

LRESULT CFilerTabGridView::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Tab
	SetFont(*(m_spGridViewProp->GetHeaderProperty()->GetFontPtr()), TRUE);

	//FilerGridView
	m_spFilerView = std::make_shared<CFilerGridView>(m_spGridViewProp);
	DWORD dwStyle = m_spFilerView->CreateWindowExArgument().dwStyle();
	m_spFilerView->CreateWindowExArgument().dwStyle(dwStyle | WS_CHILD | WS_VISIBLE);

	//Capture KeyDown Msg in FilerView
	auto fun = m_spFilerView->GetMsgHandler(WM_KEYDOWN);
	if (fun) {
		FunMsg newFun = [this, fun](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
			OnKeyDown(uMsg, wParam, lParam, bHandled);
			return fun(uMsg, wParam, lParam, bHandled);
		};
		m_spFilerView->ReplaceMsgHandler(WM_KEYDOWN, newFun);
	}

	//Folder Changed
	m_spFilerView->FolderChanged.connect([this](std::shared_ptr<CShellFolder>& pFolder) {
		for (auto i = 0; i < GetItemCount(); i++)
		{
			unsigned int id = (unsigned int)GetCurItemParam();
			if (id == (unsigned int)GetItemParam(i)) {
				SetItemText(i, pFolder->GetName().c_str());
				m_viewMap[id] = pFolder;
				break;
			}
		}
		//BOOL dummy;
		CRect rcClient = GetClientRect();
		SendMessage(WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rcClient.Width(), rcClient.Height()));
	});

	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), NM_RCLICK, &CFilerTabGridView::OnNotifyTabRClick, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_SELCHANGING, &CFilerTabGridView::OnNotifyTabSelChanging, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_SELCHANGE, &CFilerTabGridView::OnNotifyTabSelChange, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_KEYDOWN, [this](int id, LPNMHDR pnmh, BOOL& bHandled)->LRESULT {
		this->OnKeyDown(WM_KEYDOWN, (WPARAM)((NMTCKEYDOWN*)pnmh)->wVKey, NULL, bHandled);
		return 0;
	});

	m_spFilerView->Create(m_hWnd);

	if (m_vwPath.empty()) {
		//ShellFolder
		auto pFolder = std::make_shared<CShellFolder>();
		if (pFolder) {
			//New id for association
			unsigned int id = m_uniqueIDFactory.NewID();
			//CTabCtrol
			int newItem = InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, pFolder->GetName().c_str(), NULL, (LPARAM)id);
			//CFilerGridView
			m_viewMap.insert(std::make_pair(id, pFolder));
		}
	}
	else {
		for (auto path : m_vwPath) {
			//ShellFolder
			auto pFolder = std::make_shared<CShellFolder>(path);
			if (pFolder) {
				//New id for association
				unsigned int id = m_uniqueIDFactory.NewID();
				//CTabCtrol
				int newItem = InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, pFolder->GetName().c_str(), NULL, (LPARAM)id);
				//CFilerGridView
				m_viewMap.insert(std::make_pair(id, pFolder));
			}
		}
	}

	SetCurSel(0);
	unsigned int id = (unsigned int)GetCurItemParam();
	auto iter = m_viewMap.find(id);
	if (iter != m_viewMap.end()) {
		m_spFilerView->OpenFolder(iter->second);
	}
	//BOOL dummy = TRUE;
	//InvalidateRect(NULL, TRUE);
	auto rcClient = GetClientRect();
	PostMessage(WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rcClient.Width(), rcClient.Height()));

	return 0;
}

LRESULT CFilerTabGridView::OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_spFilerView->DestroyWindow();
	DestroyWindow();
	return 0;
}

LRESULT CFilerTabGridView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	bHandled = FALSE;
	int nPaddingX = GetSystemMetrics(SM_CXDLGFRAME);
	int nPaddingY = GetSystemMetrics(SM_CYDLGFRAME);

	//FilerGridView
	CRect rcTabClient = GetClientRect();
	AdjustRect(FALSE, rcTabClient);
	rcTabClient.DeflateRect(nPaddingX, nPaddingY);

	m_spFilerView->MoveWindow(rcTabClient, FALSE);
	return 0;
}

LRESULT CFilerTabGridView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (wParam)
	{
	case 'T':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			SendMessage(WM_COMMAND, IDM_NEWTAB, NULL);
		}
		break;
	case 'W':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			SendMessage(WM_COMMAND, IDM_CLOSETAB, NULL);
		}
		break;
	default:
		break;
	}
	return 0;
}


void CFilerTabGridView::AddNewView(std::wstring path)
{
	//New id for association
	unsigned int id = m_uniqueIDFactory.NewID();

	//CTabCtrol
	int newItem = InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, L"N/A", NULL, (LPARAM)id);

	//CFilerGridView
	m_viewMap.insert(std::make_pair(id, std::make_shared<CShellFolder>(path)));
	BOOL dummy = TRUE;
	OnNotifyTabSelChanging(0, NULL, dummy);
	SetCurSel(newItem);
	OnNotifyTabSelChange(0, NULL, dummy);
	auto rcClient = GetClientRect();
	SendMessage(WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rcClient.Width(), rcClient.Height()));
}



LRESULT CFilerTabGridView::OnNotifyTabSelChanging(int, LPNMHDR, BOOL& bHandled)
{
	m_prevID = (unsigned int)GetCurItemParam();
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabSelChange(int, LPNMHDR, BOOL& bHandled)
{
	unsigned int id = (unsigned int)GetCurItemParam();
	if (id != m_prevID) {
		auto iter = m_viewMap.find(id);
		if (iter != m_viewMap.end()) {
			m_spFilerView->OpenFolder(iter->second);
		}
	}
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabRClick(int id, LPNMHDR, BOOL& bHandled)
{
	CPoint ptScreen;
	::GetCursorPos(ptScreen);
	CPoint ptClient(ptScreen);
	ScreenToClient(ptClient);
	TC_HITTESTINFO tchi = { 0 };
	tchi.pt = ptClient;
	tchi.flags = TCHT_ONITEMLABEL;
	int index = HitTest(&tchi);
	SetContextMenuTabIndex(index);

	CMenu menu;
	menu.Attach(::GetSubMenu(::LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CONTEXTMENU_TAB)), 0));
	::SetForegroundWindow(m_hWnd);
	menu.TrackPopupMenu(0, ptScreen.x, ptScreen.y, m_hWnd);

	return 0;
}

LRESULT CFilerTabGridView::OnCommandNewTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//New id for association
	unsigned int id = m_uniqueIDFactory.NewID();

	//CTabControl
	int newItem = InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, L"N/A", NULL, (LPARAM)id);

	//CFilerGridView
	m_viewMap.insert(std::make_pair(id, std::make_shared<CShellFolder>()));
	BOOL dummy  = TRUE;
	OnNotifyTabSelChanging(0, NULL, dummy);
	SetCurSel(newItem);
	OnNotifyTabSelChange(0,NULL, dummy);
	OnSize(0,NULL,NULL,dummy);
	return 0;
}

LRESULT CFilerTabGridView::OnCommandCloneTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//New id for association
	unsigned int id = m_uniqueIDFactory.NewID();
	//Cloned id 
	unsigned int clonedId = (unsigned int)GetItemParam(m_contextMenuTabIndex);
	std::shared_ptr<CShellFolder> cloneFolder = m_viewMap.find(clonedId)->second->Clone();

	//CTabControl
	int newItem = InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, L"N/A", NULL, (LPARAM)id);

	//CFilerGridView
	m_viewMap.insert(std::make_pair(id, cloneFolder));

	BOOL dummy = TRUE;
	OnNotifyTabSelChanging(0, NULL, dummy);
	SetCurSel(newItem);
	OnNotifyTabSelChange(0, NULL, dummy);
	OnSize(0, NULL, NULL, dummy);
	return 0;
}


LRESULT CFilerTabGridView::OnCommandCloseTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(GetItemCount()>1){

		unsigned int id = (unsigned int)GetItemParam(m_contextMenuTabIndex);

		BOOL dummy  = FALSE;
		OnNotifyTabSelChanging(0, NULL, dummy);

		DeleteItem(m_contextMenuTabIndex);
		SetCurSel((std::min)(GetItemCount()-1, m_contextMenuTabIndex));
		m_viewMap.erase(id);

		OnNotifyTabSelChange(0, NULL, dummy);
		OnSize(0, NULL, NULL, dummy);
	}
	return 0;
}

LRESULT CFilerTabGridView::OnCommandCloseAllButThisTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (GetItemCount()>1) {

		//Remove larget tab
		for (auto index = GetItemCount() - 1; index >= 0; index--) {
			if (index != m_contextMenuTabIndex) {
				auto id = (unsigned int)GetItemParam(index);
				DeleteItem(index);
				m_viewMap.erase(id);
			}
		}

		BOOL dummy = FALSE;
		OnNotifyTabSelChanging(0, NULL, dummy);
		SetCurSel(0);
		OnNotifyTabSelChange(0, NULL, dummy);
		OnSize(0, NULL, NULL, dummy);
	}
	return 0;
}




