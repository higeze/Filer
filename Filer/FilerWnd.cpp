#include "Filer.h"
#include "FilerWnd.h"
#include "FilerGridView.h"
#include "Resource.h"

#include "ShellFile.h"
#include "ShellFolder.h"


#include "GridViewProperty.h"
#include "FavoritesProperty.h"
#include "FavoritesGridView.h"

#include "FilerProperty.h"
#include "PropertyWnd.h"


//#include "PropertyWnd.h"

CFilerWnd::CFilerWnd()
	:m_spGridViewProp(std::make_shared<CGridViewProperty>()),
	m_uniqueIDFactory(),
	m_spTab(std::make_shared<CTabCtrl>()),
	m_spFavoritesView(nullptr),
	m_viewMap(),m_rcWnd(0,0,300,500),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>())
{
	m_rca
	.lpszClassName(L"CFilerWnd")
	.style(CS_VREDRAW | CS_HREDRAW |CS_DBLCLKS)
	.hIcon((HICON)LoadImage(::GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MAINFRAME),IMAGE_ICON,0,0,0))
	.hIconSm((HICON)LoadImage(::GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MAINFRAME),IMAGE_ICON,0,0,0))
	.hCursor(::LoadCursor(NULL, IDC_ARROW))
	.hbrBackground((HBRUSH)(COLOR_3DFACE+1))
	.lpszMenuName(MAKEINTRESOURCE(IDR_MENU_FILER));
		

	m_cwa
	.lpszWindowName(L"FilerWnd") 
	.lpszClassName(L"CFilerWnd")
	.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN)
	.x(m_rcWnd.left)
	.y(m_rcWnd.top)
	.nWidth(m_rcWnd.Width())
	.nHeight(m_rcWnd.Height())
	.hMenu(NULL); 

	m_spTab->CreateWindowExArgument()
	.dwStyle(WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN | WS_VISIBLE|TCS_HOTTRACK|TCS_FLATBUTTONS|TCS_MULTILINE)
	.hMenu((HMENU)9996);



	//auto spTab = m_spTab;
	
	//m_spTab->AddMsgHandler(WM_LBUTTONDOWN, [spTab](UINT uiMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)->LRESULT{
	//	spTab->SetFocus();
	//	return 0;
	//});
	//m_spTab->AddMsgHandler(WM_KEYDOWN, &CFilerWnd::OnKeyDown, this);
	//m_spTab->AddMsgHandler(WM_RBUTTONDBLCLK, &CFilerWnd::OnTabContextMenu, this);

	AddMsgHandler(WM_CREATE,&CFilerWnd::OnCreate,this);
	AddMsgHandler(WM_SIZE,&CFilerWnd::OnSize,this);
	AddMsgHandler(WM_CLOSE,&CFilerWnd::OnClose,this);
	AddMsgHandler(WM_DESTROY,&CFilerWnd::OnDestroy,this);
	AddMsgHandler(WM_KEYDOWN,&CFilerWnd::OnKeyDown,this);
	AddCmdIDHandler(IDM_NEWTAB,&CFilerWnd::OnCommandNewTab,this);
	AddCmdIDHandler(IDM_CLOSETAB,&CFilerWnd::OnCommandCloseTab,this);

	AddNtfyHandler(9996,NM_RCLICK , &CFilerWnd::OnNotifyTabRClick, this);
	AddNtfyHandler(9996,TCN_SELCHANGING, &CFilerWnd::OnNotifyTabSelChanging, this);
	AddNtfyHandler(9996,TCN_SELCHANGE, &CFilerWnd::OnNotifyTabSelChange, this);
	AddNtfyHandler(9996,TCN_KEYDOWN, [this](int id,LPNMHDR pnmh,BOOL& bHandled)->LRESULT{
		this->OnKeyDown(WM_KEYDOWN, (WPARAM)((NMTCKEYDOWN*)pnmh)->wVKey, NULL, bHandled);
		return 0;
	});
	//AddCmdIDHandler(IDM_FILE_LOAD,&CDcmListView::OnWndCommandFileLoad,m_upList.get());
	//AddCmdIDHandler(IDM_EDIT_COPY,&CMultiLineListView::OnCmdEditCopy,(CMultiLineListView*)m_upList.get());
	//AddCmdIDHandler(IDM_EDIT_SELECTALL,&CMultiLineListView::OnCmdEditSelectAll,(CMultiLineListView*)m_upList.get());
	AddCmdIDHandler(IDM_OPTION,&CFilerWnd::OnCommandOption,this);
	AddCmdIDHandler(IDM_FAVORITESOPTION,&CFilerWnd::OnCommandFavoritesOption,this);
	//AddMsgHandler(WM_DROPFILES,&CDcmListView::OnWndDropFiles,m_upList.get());

}

CFilerWnd::~CFilerWnd(){}

HWND CFilerWnd::Create(HWND hWndParent)
{
	return CWnd::Create(hWndParent, m_rcWnd);
}

LRESULT CFilerWnd::OnCreate(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//CFavoritesGridView
	m_spFavoritesView = std::make_shared<CFavoritesGridView>(m_spGridViewProp, m_spFavoritesProp);
	m_spFavoritesView->CreateWindowExArgument()
	.dwStyle(WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN | WS_VISIBLE)
	.hMenu((HMENU)99396);

	m_spFavoritesView->FolderChoiced.connect([this](std::shared_ptr<CShellFolder>& spFolder)->void{
		if(m_spTab->GetItemCount()>0){
			unsigned int id = (unsigned int)m_spTab->GetCurItemParam();
			m_spFilerView->OpenFolder(spFolder);
		}		
	});
	m_spFavoritesView->Create(m_hWnd);

	//CTabControl
	m_spTab->Create(m_hWnd);
	m_spTab->SetFont(*(m_spGridViewProp->m_spPropHeader->GetFontPtr()), TRUE);

	//CFilerGridView
	m_spFilerView = std::make_shared<CFilerGridView>(m_spGridViewProp);
	m_spFilerView->AddMsgHandler(WM_KEYDOWN, &CFilerWnd::OnKeyDown, this);
	DWORD dwStyle = m_spFilerView->CreateWindowExArgument().dwStyle();
	m_spFilerView->CreateWindowExArgument().dwStyle(dwStyle | WS_CHILD | WS_VISIBLE);
	auto spTab = m_spTab;
	m_spFilerView->FolderChanged.connect([this](std::shared_ptr<CShellFolder>& pFolder) {
		for (auto i = 0; i<m_spTab->GetItemCount(); i++)
		{
			unsigned int id = (unsigned int)m_spTab->GetCurItemParam();
			if (id == (unsigned int)m_spTab->GetItemParam(i)) {
				m_spTab->SetItemText(i, pFolder->GetName().c_str());
				m_viewMap[id] = pFolder;
				break;
			}
		}
		BOOL dummy;
		OnSize(WM_SIZE, NULL, NULL, dummy);
	});
	m_spFilerView->Create(m_spTab->m_hWnd);
	BOOL dummy =FALSE;

	if(m_vwPath.empty()){
		OnCommandNewTab(0,0,NULL,dummy);
	}else{
		for(auto path : m_vwPath){
			//ShellFolder
			std::shared_ptr<CShellFolder> pFolder = CShellFolder::CreateShellFolderFromPath(path);
			if(pFolder){
				//New id for association
				unsigned int id = m_uniqueIDFactory.NewID();
				//CTabCtrol
				int newItem = m_spTab->InsertItem(m_spTab->GetItemCount(), TCIF_PARAM | TCIF_TEXT, pFolder->GetName().c_str(), NULL, (LPARAM)id);
				//CFilerGridView
				m_viewMap.insert(std::make_pair(id, pFolder));
			}
		}
		BOOL dummy = TRUE;
		m_spTab->SetCurSel(0);
		unsigned int id = (unsigned int)m_spTab->GetCurItemParam();
		auto iter = m_viewMap.find(id);
		if (iter != m_viewMap.end()) {
			m_spFilerView->OpenFolder(iter->second);
		}
		OnSize(0, NULL, NULL, dummy);
	}

	return 0;
}

LRESULT CFilerWnd::OnClose(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{	
	WINDOWPLACEMENT wp={0};
	wp.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	m_rcWnd=CRect(wp.rcNormalPosition);
	
	m_spTab->DestroyWindow();
	m_spFilerView->DestroyWindow();
	DestroyWindow();
	return 0;
}

LRESULT CFilerWnd::OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	::PostQuitMessage(0);
	return 0;
}

LRESULT CFilerWnd::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CRect rcClient = GetClientRect();

    int nPaddingX = GetSystemMetrics(SM_CXDLGFRAME);
    int nPaddingY = GetSystemMetrics(SM_CYDLGFRAME);

	//Favorites
	CRect rcFavoriteClient = m_spFavoritesView->GetRect();
    m_spFavoritesView->SetWindowPos(HWND_BOTTOM,
        rcClient.left , rcClient.top, 
        rcFavoriteClient.Width(), rcClient.Height(),
        SWP_SHOWWINDOW);	

	//Tab
    m_spTab->SetWindowPos(HWND_BOTTOM,
        rcClient.left + rcFavoriteClient.Width(), rcClient.top, 
        rcClient.Width() - rcFavoriteClient.Width(), rcClient.Height(),
        SWP_SHOWWINDOW);

	//FilerGridView
	CRect rcTabClient = m_spTab->GetClientRect();
	m_spTab->AdjustRect(FALSE, rcTabClient);
	rcTabClient.DeflateRect(nPaddingX, nPaddingY);

	m_spFilerView->MoveWindow(rcTabClient, FALSE);

	return 0;
}

LRESULT CFilerWnd::OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	switch (wParam)
	{
	case 'T':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SendMessage(WM_COMMAND,IDM_NEWTAB, NULL);
		}
		break;
	case 'W':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SendMessage(WM_COMMAND,IDM_CLOSETAB, NULL);
		}
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CFilerWnd::OnCommandNewTab(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	//New id for association
	unsigned int id = m_uniqueIDFactory.NewID();

	//CTabControl
	int newItem = m_spTab->InsertItem(m_spTab->GetItemCount(), TCIF_PARAM | TCIF_TEXT, L"N/A", NULL, (LPARAM)id);

	//CFilerGridView
	m_viewMap.insert(std::make_pair(id, std::make_shared<CShellFolder>()));
	BOOL dummy  = TRUE;
	OnNotifyTabSelChanging(0, NULL, dummy);
	m_spTab->SetCurSel(newItem);
	OnNotifyTabSelChange(0,NULL, dummy);
	OnSize(0,NULL,NULL,dummy);
	return 0;
}

void CFilerWnd::AddNewView(std::wstring path)
{
	//New id for association
	unsigned int id = m_uniqueIDFactory.NewID();

	//CTabCtrol
	int newItem = m_spTab->InsertItem(m_spTab->GetItemCount(), TCIF_PARAM | TCIF_TEXT, L"N/A", NULL, (LPARAM)id);

	//CFilerGridView
	m_viewMap.insert(std::make_pair(id, CShellFolder::CreateShellFolderFromPath(path)));
	BOOL dummy  = TRUE;
	OnNotifyTabSelChanging(0, NULL, dummy);
	m_spTab->SetCurSel(newItem);
	OnNotifyTabSelChange(0,NULL, dummy);
	OnSize(0,NULL,NULL,dummy);
}

LRESULT CFilerWnd::OnCommandCloseTab(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	if(m_spTab->GetItemCount()>1){
		//unsigned int id = (unsigned int)m_spTab->GetCurItemParam();

		unsigned int id = (unsigned int)m_spTab->GetItemParam(m_contextMenuTabIndex);

		BOOL dummy  = FALSE;
		OnNotifyTabSelChanging(0, NULL, dummy);

		m_spTab->DeleteItem(m_contextMenuTabIndex);
		m_spTab->SetCurSel(m_spTab->GetItemCount()-1);
		m_viewMap.erase(id);

		OnNotifyTabSelChange(0, NULL, dummy);
	}
	return 0;
}

LRESULT CFilerWnd::OnCommandOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	CRect rc(0,0,0,0);

	auto pPropWnd=new CPropertyWnd<CGridViewProperty>(
		m_spGridViewProp->m_spBackgroundProperty,
		m_spGridViewProp->m_spPropHeader,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropHeader,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spDeltaScroll,
		L"GridViewProperty",
		m_spGridViewProp);

	pPropWnd->PropertyChanged.connect([this](const std::wstring& str)->void{
		for(auto iter = m_viewMap.begin();iter!= m_viewMap.end();++iter){
			m_spFilerView->UpdateAll();
		}
		m_spFavoritesView->UpdateAll();
		//SerializeProperty(m_spGridViewProp);
	});
	//pPropWnd->PropertyChanged.connect([&](const std::wstring& str)->void{
	//	m_gridView.GetChartColumnPtr()->SetVisible(*(m_spPropDcmDesigner->GetColumnPropertyPtr()->m_visibleChartColumnPtr));
	//	m_gridView.GetCompareColumnPtr()->SetVisible(*(m_spPropDcmDesigner->GetColumnPropertyPtr()->m_visibleCompareColumnPtr));
	//	m_gridView.GetLangColumnPtr()->SetVisible(*(m_spPropDcmDesigner->GetColumnPropertyPtr()->m_visibleLangColumnPtr));
	//	m_gridView.GetTypeGColumnPtr()->SetVisible(*(m_spPropDcmDesigner->GetColumnPropertyPtr()->m_visibleTypeGColumnPtr));
	//	m_gridView.GetTypeEColumnPtr()->SetVisible(*(m_spPropDcmDesigner->GetColumnPropertyPtr()->m_visibleTypeEColumnPtr));

	//	m_gridView.UpdateAll();
	//	SerializeProperty(m_spPropDcmDesigner);
	//});
	pPropWnd->Create(m_hWnd,rc);
	rc=CRect(pPropWnd->GetGridView()->MeasureSize());
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, 0);
	pPropWnd->MoveWindow(0,0,rc.Width()+::GetSystemMetrics(SM_CXVSCROLL),min(500,rc.Height()+::GetSystemMetrics(SM_CYVSCROLL)+10),FALSE);
	pPropWnd->CenterWindow();
	pPropWnd->ShowWindow(SW_SHOW);
	pPropWnd->UpdateWindow();

	return 0;
}

LRESULT CFilerWnd::OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	CRect rc(0,0,0,0);

	auto pPropWnd=new CPropertyWnd<CFavoritesProperty>(
		m_spGridViewProp->m_spBackgroundProperty,
		m_spGridViewProp->m_spPropHeader,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropHeader,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spDeltaScroll,
		L"FavoritesProperty",
		m_spFavoritesProp);

	pPropWnd->PropertyChanged.connect([&](const std::wstring& str)->void{
		m_spFavoritesView->OpenFavorites();
		m_spFavoritesView->UpdateAll();
		//SerializeProperty();
	});

	pPropWnd->Create(m_hWnd,rc);
	rc=CRect(pPropWnd->GetGridView()->MeasureSize());
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, 0);
	pPropWnd->MoveWindow(0,0,rc.Width()+::GetSystemMetrics(SM_CXVSCROLL),min(500,rc.Height()+::GetSystemMetrics(SM_CYVSCROLL)+10),FALSE);
	pPropWnd->CenterWindow();
	pPropWnd->ShowWindow(SW_SHOW);
	pPropWnd->UpdateWindow();

	return 0;
}



LRESULT CFilerWnd::OnNotifyTabSelChanging(int, LPNMHDR, BOOL& bHandled)
{
	m_prevID = (unsigned int)m_spTab->GetCurItemParam();
	return 0;
}

LRESULT CFilerWnd::OnNotifyTabSelChange(int, LPNMHDR, BOOL& bHandled)
{
	unsigned int id = (unsigned int)m_spTab->GetCurItemParam();
	if (id != m_prevID) {
		auto iter = m_viewMap.find(id);
		if (iter != m_viewMap.end()) {
			m_spFilerView->OpenFolder(iter->second);
		}
	}
	return 0;
}

LRESULT CFilerWnd::OnNotifyTabRClick(int id, LPNMHDR, BOOL& bHandled)
{
	CPoint ptScreen;	
	::GetCursorPos(ptScreen);
	CPoint ptClient(ptScreen);
	m_spTab->ScreenToClient(ptClient);
	TC_HITTESTINFO tchi = {0};
	tchi.pt = ptClient;
	tchi.flags = TCHT_ONITEMLABEL;
	int index = m_spTab->HitTest(&tchi);
	SetContextMenuTabIndex(index);

	CMenu menu;
	menu.Attach(::GetSubMenu(::LoadMenu(::GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_CONTEXTMENU_TAB)), 0));	
			::SetForegroundWindow(m_hWnd);
			menu.TrackPopupMenu(0,ptScreen.x,ptScreen.y,m_hWnd);

	return 0;
}
