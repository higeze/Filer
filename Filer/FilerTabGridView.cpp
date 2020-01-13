#include "FilerTabGridView.h"
#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "FilerWnd.h"
#include "FavoriteRow.h"
#include "KnownFolder.h"
#include "ShellFileFactory.h"
#include "Debug.h"

CFilerTabGridView::CFilerTabGridView(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:m_spFilerView(std::make_shared<CFilerGridView>(spFilerGridViewProp))
{
	CreateWindowExArgument()
		.dwStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | TCS_HOTTRACK | TCS_FLATBUTTONS | TCS_MULTILINE);

	//FilerGridView
	DWORD dwStyle = m_spFilerView->CreateWindowExArgument().dwStyle();
	m_spFilerView->CreateWindowExArgument().dwStyle(dwStyle | WS_CHILD | WS_VISIBLE);

	AddMsgHandler(WM_CREATE, &CFilerTabGridView::OnCreate, this);
	AddMsgHandler(WM_SIZE, &CFilerTabGridView::OnSize, this);
	AddMsgHandler(WM_CLOSE, &CFilerTabGridView::OnClose, this);
	//AddMsgHandler(WM_DESTROY, &CFilerTabGridView::OnDestroy, this);
	AddMsgHandler(WM_KEYDOWN,&CFilerTabGridView::OnKeyDown,this);

	AddCmdIDHandler(IDM_NEWTAB, &CFilerTabGridView::OnCommandNewTab, this);
	AddCmdIDHandler(IDM_CLONETAB, &CFilerTabGridView::OnCommandCloneTab, this);
	AddCmdIDHandler(IDM_CLOSETAB, &CFilerTabGridView::OnCommandCloseTab, this);
	AddCmdIDHandler(IDM_CLOSEALLBUTTHISTAB, &CFilerTabGridView::OnCommandCloseAllButThisTab, this);
	AddCmdIDHandler(IDM_ADDTOFAVORITE, &CFilerTabGridView::OnCommandAddToFavorite, this);
	AddCmdIDHandler(IDM_OPENSAMEASOTHER, &CFilerTabGridView::OnCommandOpenSameAsOther, this);
}

LRESULT CFilerTabGridView::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_selectedIndex.Changed.connect([this](const NotifyChangedEventArgs<int>& e)->void {
		SetCurSel(std::clamp(m_selectedIndex.get(), 0, GetItemCount() - 1));
		if (m_spFilerView->IsWindow()) {
			m_spFilerView->OpenFolder(m_folders[m_selectedIndex.get()]);
		}
		BOOL dummy = FALSE;
		OnSize(0, NULL, NULL, dummy);
	});

	m_folders.VectorChanged.connect([this](const NotifyVectorChangedEventArgs<std::shared_ptr<CShellFolder>>& e)->void {
		switch (e.Action) {
		case NotifyVectorChangedAction::Add:
		{
			for (auto& item : e.NewItems) {
				InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, e.NewItems[0]->GetFileNameWithoutExt().c_str(), NULL, NULL);
			}
		}
		break;
		case NotifyVectorChangedAction::Move:
			break;
		case NotifyVectorChangedAction::Remove:
		{
			for (int i = e.OldStartingIndex + e.OldItems.size() - 1; i >= e.OldStartingIndex; i--) {
				DeleteItem(i);
			}
		}
		break;

		case NotifyVectorChangedAction::Replace:
		case NotifyVectorChangedAction::Reset:
		default:
			break;
		}

	});

	//Tab
	m_font = m_spFilerView->GetHeaderProperty()->Format->Font.GetGDIFont();
	SetFont(m_font, TRUE);

	//Capture KeyDown Msg in FilerView
	auto fun = m_spFilerView->GetMsgHandler(WM_KEYDOWN);
	if (fun) {
		FunMsg newFun = [this, fun](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
			LRESULT lr = OnKeyDown(uMsg, wParam, lParam, bHandled);
			if (bHandled) {
				return lr;
			}else{
				return fun(uMsg, wParam, lParam, bHandled);
			}
		};
		m_spFilerView->ReplaceMsgHandler(WM_KEYDOWN, newFun);
	}

	//Folder Changed
	m_spFilerView->FolderChanged.connect([this](std::shared_ptr<CShellFolder>& pFolder) {
		m_folders[GetCurSel()] = pFolder;
		SetItemText(GetCurSel(), m_folders[GetCurSel()]->GetFileNameWithoutExt().c_str());
		int nPaddingX = GetSystemMetrics(SM_CXDLGFRAME);
		int nPaddingY = GetSystemMetrics(SM_CYDLGFRAME);

		//FilerGridView
		CRect rcTabClient = GetClientRect();
		AdjustRect(FALSE, rcTabClient);
		rcTabClient.DeflateRect(nPaddingX, nPaddingY);

		m_spFilerView->PostUpdate(Updates::Rect);
		m_spFilerView->SetUpdateRect(m_spFilerView->GetDirectPtr()->Pixels2Dips(rcTabClient));
	});

	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), NM_CLICK, &CFilerTabGridView::OnNotifyTabLClick, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), NM_RCLICK, &CFilerTabGridView::OnNotifyTabRClick, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_SELCHANGING, &CFilerTabGridView::OnNotifyTabSelChanging, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_SELCHANGE, &CFilerTabGridView::OnNotifyTabSelChange, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_KEYDOWN, [this](int id, LPNMHDR pnmh, BOOL& bHandled)->LRESULT {
		this->OnKeyDown(WM_KEYDOWN, (WPARAM)((NMTCKEYDOWN*)pnmh)->wVKey, NULL, bHandled);
		return 0;
	});

	//Tabs
	for (auto path : m_vwPath) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
		if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
			m_folders.notify_push_back(spFolder);
		} else {
			m_folders.notify_push_back(CKnownFolderManager::GetInstance()->GetDesktopFolder());
		}
	}

	if (m_folders.empty()) {
		m_folders.notify_push_back(CKnownFolderManager::GetInstance()->GetDesktopFolder());
	}

	SetCurSel(std::clamp(m_selectedIndex.get(), 0, GetItemCount() -1));

	//FilerGridView
	//Size
	int nPaddingX = GetSystemMetrics(SM_CXDLGFRAME);
	int nPaddingY = GetSystemMetrics(SM_CYDLGFRAME);
	CRect rcTabClient = GetClientRect();
	AdjustRect(FALSE, rcTabClient);
	rcTabClient.DeflateRect(nPaddingX, nPaddingY);
	//Create
	m_spFilerView->Create(m_hWnd, rcTabClient);
	//Open
	m_spFilerView->OpenFolder(m_folders[GetCurSel()]);

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
	if (m_spFilerView->IsWindow()) {
		//int width = LOWORD(lParam);
		//int height = HIWORD(lParam);
		int nPaddingX = GetSystemMetrics(SM_CXDLGFRAME);
		int nPaddingY = GetSystemMetrics(SM_CYDLGFRAME);

		//FilerGridView
		CRect rcTabClient = GetClientRect();
		AdjustRect(FALSE, rcTabClient);
		rcTabClient.DeflateRect(nPaddingX, nPaddingY);

		m_spFilerView->MoveWindow(rcTabClient, FALSE);
	}
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
		bHandled = TRUE;
		break;
	case 'W':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			SendMessage(WM_COMMAND, IDM_CLOSETAB, NULL);
		}
		bHandled = TRUE;
		break;
	default:
		break;
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabSelChanging(int, LPNMHDR, BOOL& bHandled)
{
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabSelChange(int, LPNMHDR, BOOL& bHandled)
{
	m_selectedIndex.notify_set(GetCurSel());
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabLClick(int id, LPNMHDR, BOOL& bHandled)
{
	::SetFocus(m_spFilerView->m_hWnd);
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
	BOOL dummy = FALSE;
	OnNotifyTabSelChanging(0, NULL, dummy);

	m_folders.notify_push_back(CKnownFolderManager::GetInstance()->GetDesktopFolder());

	SetCurSel(GetItemCount() - 1);
	OnNotifyTabSelChange(0, NULL, dummy);
	OnSize(0, NULL, NULL, dummy);

	return 0;
}

LRESULT CFilerTabGridView::OnCommandCloneTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BOOL dummy = FALSE;
	OnNotifyTabSelChanging(0, NULL, dummy);

	m_folders.notify_push_back(m_folders[m_contextMenuTabIndex]->Clone());

	SetCurSel(GetItemCount() - 1);
	OnNotifyTabSelChange(0, NULL, dummy);
	OnSize(0, NULL, NULL, dummy);
	return 0;
}

LRESULT CFilerTabGridView::OnCommandCloseTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(GetItemCount()>1){
		BOOL dummy = FALSE;
		OnNotifyTabSelChanging(0, NULL, dummy);

		m_folders.notify_erase(m_folders.begin() + m_contextMenuTabIndex);

		SetCurSel((std::min)(GetItemCount() - 1, m_contextMenuTabIndex));
		OnNotifyTabSelChange(0, NULL, dummy);
		OnSize(0, NULL, NULL, dummy);
	}
	return 0;
}

LRESULT CFilerTabGridView::OnCommandCloseAllButThisTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (GetItemCount()>1) {
		BOOL dummy = FALSE;
		OnNotifyTabSelChanging(0, NULL, dummy);

		//Erase larger tab
		m_folders.notify_erase(m_folders.begin() + (m_contextMenuTabIndex + 1), m_folders.end());
		//Erase smaller tab
		m_folders.notify_erase(m_folders.begin(), m_folders.begin() + m_contextMenuTabIndex);

		SetCurSel(0);
		OnNotifyTabSelChange(0, NULL, dummy);
		OnSize(0, NULL, NULL, dummy);
	}
	return 0;
}

LRESULT CFilerTabGridView::OnCommandAddToFavorite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if(auto p = dynamic_cast<CFilerWnd*>(m_pParentWnd)){
		p->GetFavoritesPropPtr()->GetFavorites()->push_back(std::make_shared<CFavorite>(m_folders[m_contextMenuTabIndex]->GetPath(), L""));
		p->GetLeftFavoritesView()->PushRow(std::make_shared<CFavoriteRow>(p->GetLeftFavoritesView().get(), p->GetFavoritesPropPtr()->GetFavorites()->size() - 1));
		p->GetRightFavoritesView()->PushRow(std::make_shared<CFavoriteRow>(p->GetRightFavoritesView().get(), p->GetFavoritesPropPtr()->GetFavorites()->size() - 1));
		p->GetLeftFavoritesView()->SubmitUpdate();
		p->GetRightFavoritesView()->SubmitUpdate();
	}
	return 0;
}

LRESULT CFilerTabGridView::OnCommandOpenSameAsOther(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if (auto p = dynamic_cast<CFilerWnd*>(m_pParentWnd)) {
			
		std::shared_ptr<CFilerTabGridView> otherView;

		if (this == p->GetLeftView().get()) {
			otherView = p->GetRightView();
		}
		else {
			otherView = p->GetLeftView();
		}
			
		GetGridView()->OpenFolder(otherView->GetGridView()->GetFolder());

	}
	return 0;
}




