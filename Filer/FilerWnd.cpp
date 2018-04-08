#include "Filer.h"
#include "FilerWnd.h"
#include "FilerGridView.h"
#include "Resource.h"

#include "ShellFile.h"
#include "ShellFolder.h"


#include "GridViewProperty.h"
#include "FavoritesProperty.h"
#include "FavoritesGridView.h"
#include "FavoriteRow.h"

#include "FilerProperty.h"
#include "PropertyWnd.h"

#include "FilerTabGridView.h"


//#include "PropertyWnd.h"

//CUniqueIDFactory CFilerWnd::ControlIDFactory = CUniqueIDFactory(9996);

CFilerWnd::CFilerWnd()
	:m_spGridViewProp(std::make_shared<CGridViewProperty>()),
	//m_uniqueIDFactory(),
	//m_spTab1(std::make_shared<CTabCtrl>()),
	//m_spTab2(std::make_shared<CTabCtrl>()),
	//m_spFavoritesView(nullptr),
	//m_spLeftView(std::make_shared<CFilerTabGridView>()),
	//m_spLeftView(std::make_shared<CFilerTabGridView>()),
	//m_viewMap(),
	m_rcWnd(0,0,300,500),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	m_spApplicationProp(std::make_shared<CApplicationProperty>())
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

	//m_spTab1->CreateWindowExArgument()
	//.dwStyle(WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN | WS_VISIBLE|TCS_HOTTRACK|TCS_FLATBUTTONS|TCS_MULTILINE)
	//.hMenu((HMENU)9996);

	//m_spTab2->CreateWindowExArgument()
	//	.dwStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | TCS_HOTTRACK | TCS_FLATBUTTONS | TCS_MULTILINE)
	//	.hMenu((HMENU)9997);

	AddMsgHandler(WM_CREATE,&CFilerWnd::OnCreate,this);
	AddMsgHandler(WM_SIZE,&CFilerWnd::OnSize,this);
	AddMsgHandler(WM_CLOSE,&CFilerWnd::OnClose,this);
	AddMsgHandler(WM_DESTROY,&CFilerWnd::OnDestroy,this);
	//AddMsgHandler(WM_KEYDOWN,&CFilerWnd::OnKeyDown,this);

	//AddCmdIDHandler(IDM_NEWTAB,&CFilerWnd::OnCommandNewTab,this);
	//AddCmdIDHandler(IDM_CLONETAB, &CFilerWnd::OnCommandCloneTab, this);
	//AddCmdIDHandler(IDM_CLOSETAB,&CFilerWnd::OnCommandCloseTab,this);
	//AddCmdIDHandler(IDM_CLOSEALLBUTTHISTAB, &CFilerWnd::OnCommandCloseAllButThisTab, this);
	//AddCmdIDHandler(IDM_ADDTOFAVORITE, &CFilerWnd::OnCommandAddToFavorite, this);

	//AddNtfyHandler(9996,NM_RCLICK , &CFilerWnd::OnNotifyTabRClick, this);
	//AddNtfyHandler(9996,TCN_SELCHANGING, &CFilerWnd::OnNotifyTabSelChanging, this);
	//AddNtfyHandler(9996,TCN_SELCHANGE, &CFilerWnd::OnNotifyTabSelChange, this);
	//AddNtfyHandler(9996,TCN_KEYDOWN, [this](int id,LPNMHDR pnmh,BOOL& bHandled)->LRESULT{
	//	this->OnKeyDown(WM_KEYDOWN, (WPARAM)((NMTCKEYDOWN*)pnmh)->wVKey, NULL, bHandled);
	//	return 0;
	//});

	//AddCmdIDHandler(IDM_FILE_LOAD,&CDcmListView::OnWndCommandFileLoad,m_upList.get());
	//AddCmdIDHandler(IDM_EDIT_COPY,&CMultiLineListView::OnCmdEditCopy,(CMultiLineListView*)m_upList.get());
	//AddCmdIDHandler(IDM_EDIT_SELECTALL,&CMultiLineListView::OnCmdEditSelectAll,(CMultiLineListView*)m_upList.get());
	AddCmdIDHandler(IDM_APPLICATIONOPTION, &CFilerWnd::OnCommandApplicationOption, this);
	AddCmdIDHandler(IDM_GRIDVIEWOPTION,&CFilerWnd::OnCommandGridViewOption,this);
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

	m_spFavoritesView->FileChosen.connect([this](std::shared_ptr<CShellFile>& spFile)->void{
		if(m_spCurView->GetItemCount()>0){
			m_spCurView->GetGridView()->Open(spFile);
		}		
	});
	m_spFavoritesView->Create(m_hWnd);
	//Left Right View
	m_spCurView = m_spLeftView;

	m_spLeftView->Create(m_hWnd);
	m_spRightView->Create(m_hWnd);
	
	m_spLeftView->SubclassWindow(m_spLeftView->m_hWnd);
	m_spRightView->SubclassWindow(m_spRightView->m_hWnd);

	BOOL dummy = FALSE;
	m_spLeftView->OnCreate(WM_CREATE, NULL, NULL, dummy);
	m_spRightView->OnCreate(WM_CREATE, NULL, NULL, dummy);

	m_spLeftView->AddMsgHandler(WM_SETFOCUS,
		[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	{ m_spCurView = m_spLeftView; return 0; });
	m_spLeftView->GetGridView()->AddMsgHandler(WM_SETFOCUS,
		[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	{ m_spCurView = m_spLeftView; return 0; });
	m_spRightView->AddMsgHandler(WM_SETFOCUS,
		[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	{ m_spCurView = m_spRightView; return 0; });
	m_spRightView->GetGridView()->AddMsgHandler(WM_SETFOCUS,
		[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	{ m_spCurView = m_spRightView; return 0; });


	auto applyCustomContextMenu = [this](std::shared_ptr<CFilerGridView> spFilerView)->void {

		spFilerView->AddCustomContextMenu = [&](CMenu& menu) {
			menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);
			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_TYPE | MIIM_ID;
			mii.fType = MFT_STRING;
			mii.fState = MFS_ENABLED;
			mii.wID = IDM_ADDTOFAVORITEINGRID;
			mii.dwTypeData = L"Add to favorite";
			menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
		};

		spFilerView->ExecCustomContextMenu = [&](int idCmd, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl)->bool {
			if (idCmd == IDM_ADDTOFAVORITEINGRID) {
				for (auto& pIdl : vpIdl) {
					STRRET strret;
					psf->GetDisplayNameOf(pIdl, SHGDN_FORPARSING, &strret);
					std::wstring path = STRRET2WSTR(strret, pIdl);
					m_spFavoritesProp->GetFavorites()->push_back(CFavorite(path, L""));
					m_spFavoritesView->InsertRow(CRow::kMaxIndex, std::make_shared<CFavoriteRow>(m_spFavoritesView.get(), m_spFavoritesProp->GetFavorites()->size() - 1));
				}
				m_spFavoritesView->SubmitUpdate();
				return true;
			}
			else {
				return false;
			}
		};
	};

	applyCustomContextMenu(m_spLeftView->GetGridView());
	applyCustomContextMenu(m_spRightView->GetGridView());

	auto rcClient = GetClientRect();
	PostMessage(WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rcClient.Width(), rcClient.Height()));

	return 0;
}

LRESULT CFilerWnd::OnClose(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{	
	WINDOWPLACEMENT wp={0};
	wp.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	m_rcWnd=CRect(wp.rcNormalPosition);
	
	m_spLeftView->DestroyWindow();
	m_spRightView->DestroyWindow();
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

	//Favorites
	CRect rcFavoriteClient = m_spFavoritesView->GetRect();
    m_spFavoritesView->SetWindowPos(HWND_BOTTOM,
        rcClient.left , rcClient.top, 
        rcFavoriteClient.Width(), rcClient.Height(),
        SWP_SHOWWINDOW);

	if (rcClient.Width() >= 800) {
		m_spLeftView->SetWindowPos(HWND_BOTTOM,
			rcClient.left + rcFavoriteClient.Width(), rcClient.top,
			(rcClient.Width() - rcFavoriteClient.Width())/2, rcClient.Height(),
			SWP_SHOWWINDOW);
		m_spRightView->SetWindowPos(HWND_BOTTOM,
			rcClient.left + rcFavoriteClient.Width() + (rcClient.Width() - rcFavoriteClient.Width()) / 2, rcClient.top,
			(rcClient.Width() - rcFavoriteClient.Width()) / 2, rcClient.Height(),
			SWP_SHOWWINDOW);
	}else{
		m_spLeftView->SetWindowPos(HWND_BOTTOM,
			rcClient.left + rcFavoriteClient.Width(), rcClient.top,
			rcClient.Width() - rcFavoriteClient.Width(), rcClient.Height(),
			SWP_SHOWWINDOW);
		m_spRightView->ShowWindow(SW_HIDE);
	}

	return 0;
}

//LRESULT CFilerWnd::OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
//{
//	switch (wParam)
//	{
//	case 'T':
//		if(::GetAsyncKeyState(VK_CONTROL)){
//			SendMessage(WM_COMMAND,IDM_NEWTAB, NULL);
//		}
//		break;
//	case 'W':
//		if(::GetAsyncKeyState(VK_CONTROL)){
//			SendMessage(WM_COMMAND,IDM_CLOSETAB, NULL);
//		}
//		break;
//	default:
//		break;
//	}
//	return 0;
//}



LRESULT CFilerWnd::OnCommandApplicationOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CRect rc(0, 0, 0, 0);

	auto pPropWnd = new CPropertyWnd<CApplicationProperty>(
		m_spGridViewProp->m_spBackgroundProperty,
		m_spGridViewProp->m_spPropHeader,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropHeader,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spPropCell,
		m_spGridViewProp->m_spDeltaScroll,
		L"GridViewProperty",
		m_spApplicationProp);

	pPropWnd->PropertyChanged.connect([this](const std::wstring& str)->void {
		m_spLeftView->GetGridView()->UpdateAll();
		m_spRightView->GetGridView()->UpdateAll();
		m_spFavoritesView->UpdateAll();
		SerializeProperty(this);
	});

	pPropWnd->Create(m_hWnd, rc);
	rc = CRect(pPropWnd->GetGridView()->MeasureSize());
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, 0);
	pPropWnd->MoveWindow(0, 0, rc.Width() + ::GetSystemMetrics(SM_CXVSCROLL), min(500, rc.Height() + ::GetSystemMetrics(SM_CYVSCROLL) + 10), FALSE);
	pPropWnd->CenterWindow();
	pPropWnd->ShowWindow(SW_SHOW);
	pPropWnd->UpdateWindow();

	return 0;
}


LRESULT CFilerWnd::OnCommandGridViewOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
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
		m_spLeftView->GetGridView()->UpdateAll();
		m_spRightView->GetGridView()->UpdateAll();
		m_spFavoritesView->UpdateAll();
		SerializeProperty(this);
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
		SerializeProperty(this);
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

