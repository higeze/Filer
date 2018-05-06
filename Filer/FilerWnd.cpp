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

CFilerWnd::CFilerWnd()
	:m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	m_rcWnd(0, 0, 300, 500),
	m_spGridViewProp(std::make_shared<CGridViewProperty>()),
	m_spFavoritesView(std::make_shared<CFavoritesGridView>(m_spGridViewProp)),
	m_spLeftView(std::make_shared<CFilerTabGridView>(m_spGridViewProp)),
	m_spRightView(std::make_shared<CFilerTabGridView>(m_spGridViewProp)),
	m_spCurView(m_spLeftView)
{
	m_spLeftView->SetParentWnd(this);
	m_spLeftView->CreateWindowExArgument().hMenu((HMENU)9996);

	m_spRightView->SetParentWnd(this);
	m_spLeftView->CreateWindowExArgument().hMenu((HMENU)9997);

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

	AddMsgHandler(WM_CREATE,&CFilerWnd::OnCreate,this);
	AddMsgHandler(WM_SIZE,&CFilerWnd::OnSize,this);
	AddMsgHandler(WM_CLOSE,&CFilerWnd::OnClose,this);
	AddMsgHandler(WM_DESTROY,&CFilerWnd::OnDestroy,this);
	AddMsgHandler(WM_SETFOCUS, &CFilerWnd::OnSetFocus, this);

	AddMsgHandler(WM_LBUTTONDOWN, &CFilerWnd::OnLButtonDown, this);
	AddMsgHandler(WM_LBUTTONUP, &CFilerWnd::OnLButtonUp, this);
	AddMsgHandler(WM_MOUSEMOVE, &CFilerWnd::OnMouseMove, this);
	//AddMsgHandler(WM_KEYDOWN,&CFilerWnd::OnKeyDown,this);
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
	AddCmdIDHandler(IDM_LEFTVIEWOPTION, &CFilerWnd::OnCommandLeftViewOption, this);
	AddCmdIDHandler(IDM_RIGHTVIEWOPTION, &CFilerWnd::OnCommandRightViewOption, this);
}

CFilerWnd::~CFilerWnd(){}

HWND CFilerWnd::Create(HWND hWndParent)
{
	return CWnd::Create(hWndParent, m_rcWnd);
}

LRESULT CFilerWnd::OnCreate(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//CFavoritesGridView
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

	//Capture KeyDown Msg in FilerView
	auto fun = m_spLeftView->GetGridView()->GetMsgHandler(WM_KEYDOWN);
	if (fun) {
		FunMsg newFun = [this, fun](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
			LRESULT lr = OnKeyDown(uMsg, wParam, lParam, bHandled);
			if (bHandled) {
				return lr;
			}else {
				return fun(uMsg, wParam, lParam, bHandled);
			}
		};
		m_spLeftView->GetGridView()->ReplaceMsgHandler(WM_KEYDOWN, newFun);
	}

	fun = m_spRightView->GetGridView()->GetMsgHandler(WM_KEYDOWN);
	if (fun) {
		FunMsg newFun = [this, fun](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
			LRESULT lr = OnKeyDown(uMsg, wParam, lParam, bHandled);
			if (bHandled) {
				return lr;
			}else {
				return fun(uMsg, wParam, lParam, bHandled);
			}
		};
		m_spRightView->GetGridView()->ReplaceMsgHandler(WM_KEYDOWN, newFun);
	}


	//m_spLeftView->AddMsgHandler(WM_SETFOCUS,
	//	[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	//{ ::SetFocus(m_spLeftView->GetGridView()->m_hWnd); return 0; });

	m_spLeftView->GetGridView()->AddMsgHandler(WM_SETFOCUS,
		[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	{ 
		m_spCurView = m_spLeftView;
		m_spCurView->GetGridView()->InvalidateRect(NULL, FALSE);
		return 0;
	});

	//m_spRightView->AddMsgHandler(WM_SETFOCUS,
	//	[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	//{ ::SetFocus(m_spRightView->GetGridView()->m_hWnd); return 0; });

	m_spRightView->GetGridView()->AddMsgHandler(WM_SETFOCUS,
		[this](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT
	{
		m_spCurView = m_spRightView;
		m_spCurView->GetGridView()->InvalidateRect(NULL, FALSE);
		return 0;
	});


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
					m_spFavoritesView->GetFavoritesProp()->GetFavorites()->push_back(CFavorite(path, L""));
					m_spFavoritesView->InsertRow(CRow::kMaxIndex, std::make_shared<CFavoriteRow>(m_spFavoritesView.get(), m_spFavoritesView->GetFavoritesProp()->GetFavorites()->size() - 1));
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

	CRect rcClient = GetClientRect();
	if (m_rcLeft.left == 0 && m_rcLeft.right == 0 && m_rcLeft.top == 0 && m_rcLeft.bottom == 0) {
		//Initialize window position
		CRect rcFavoriteClient = m_spFavoritesView->GetRect();
		m_spFavoritesView->SetWindowPos(HWND_BOTTOM,
			rcClient.left, rcClient.top,
			rcFavoriteClient.Width(), rcClient.Height(),
			SWP_SHOWWINDOW);

		if (rcClient.Width() >= 800) {
			m_spLeftView->SetWindowPos(HWND_BOTTOM,
				rcClient.left + rcFavoriteClient.Width(), rcClient.top,
				(rcClient.Width() - rcFavoriteClient.Width() - kSplitterWidth) / 2, rcClient.Height(),
				SWP_SHOWWINDOW);
			m_spRightView->SetWindowPos(HWND_BOTTOM,
				rcClient.left + rcFavoriteClient.Width() + kSplitterWidth + (rcClient.Width() - rcFavoriteClient.Width() - kSplitterWidth) / 2, rcClient.top,
				(rcClient.Width() - rcFavoriteClient.Width()) / 2, rcClient.Height(),
				SWP_SHOWWINDOW);
		}
		else {
			m_spLeftView->SetWindowPos(HWND_BOTTOM,
				rcClient.left + rcFavoriteClient.Width(), rcClient.top,
				rcClient.Width() - rcFavoriteClient.Width(), rcClient.Height(),
				SWP_SHOWWINDOW);
			m_spRightView->ShowWindow(SW_HIDE);
		}
		CRect rcc(m_spLeftView->GetClientRect());
		m_rcLeft = ScreenToClientRect(m_spLeftView->GetWindowRect());
	}
	else {
		PostMessage(WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rcClient.Width(), rcClient.Height()));
	}


	return 0;
}

LRESULT CFilerWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (wParam)
	{
	case VK_F5:
		{
			int okcancel = ::MessageBox(m_hWnd, L"Copy?", L"Copy?", MB_OKCANCEL);
			if (okcancel == IDOK) {
				std::shared_ptr<CFilerTabGridView> spOtherView = m_spCurView == m_spLeftView ? m_spRightView : m_spLeftView;
				CComPtr<IShellItem2> pDestShellItem;

				HRESULT hr = ::SHCreateItemFromIDList(spOtherView->GetGridView()->GetFolder()->GetAbsolutePidl(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem));
				if (FAILED(hr)) { break; }
				m_spCurView->GetGridView()->CopyTo(pDestShellItem);
			}
			bHandled = TRUE;
		}
		break;
	case VK_F6:
		{
			int okcancel = ::MessageBox(m_hWnd, L"Move?", L"Move?", MB_OKCANCEL);
			if (okcancel == IDOK) {
				std::shared_ptr<CFilerTabGridView> spOtherView = m_spCurView == m_spLeftView ? m_spRightView : m_spLeftView;
				CComPtr<IShellItem2> pDestShellItem;

				HRESULT hr = ::SHCreateItemFromIDList(spOtherView->GetGridView()->GetFolder()->GetAbsolutePidl(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem));
				if (FAILED(hr)) { break; }
				m_spCurView->GetGridView()->MoveTo(pDestShellItem);
			}
			bHandled = TRUE;
	}
		break;
	default:
		break;
	}
	bHandled = FALSE;
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

LRESULT CFilerWnd::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint pt((short)LOWORD(lParam), (short)HIWORD(lParam));	
	CRect leftRc(ScreenToClientRect(m_spLeftView->GetWindowRect()));

	m_isSizing = (pt.x >= (leftRc.right)) &&
		(pt.x <= (leftRc.right + kSplitterWidth));

	if (m_isSizing)
	{
		m_ptStart = pt;
		SetCapture();
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	}



	return 0;
}

LRESULT CFilerWnd::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	if (m_isSizing)
	{
		m_ptStart.SetPoint(0, 0);
		ReleaseCapture();
		m_isSizing = false;
	}
	return 0;
}

LRESULT CFilerWnd::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint pt((short)LOWORD(lParam), (short)HIWORD(lParam));
	CRect leftRc(ScreenToClientRect(m_spLeftView->GetWindowRect()));

	if ((pt.x >= (leftRc.right)) &&
		(pt.x <= (leftRc.right + kSplitterWidth)))
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	}

	if (m_isSizing && wParam == MK_LBUTTON)
	{
		m_rcLeft = leftRc;
		m_rcLeft.right += pt.x - m_ptStart.x;
		m_ptStart = pt;
		PostMessage(WM_SIZE, 0, 0);
	}

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

	if (rcClient.Width() >= m_rcLeft.Width()) {
		m_spLeftView->SetWindowPos(HWND_BOTTOM,
			m_rcLeft.left, m_rcLeft.top, m_rcLeft.Width(), rcClient.Height(),
			SWP_SHOWWINDOW);
		m_spLeftView->UpdateWindow();
		m_spRightView->SetWindowPos(HWND_BOTTOM,
			m_rcLeft.right + kSplitterWidth, rcClient.top,
			rcClient.right - (m_rcLeft.right + kSplitterWidth), rcClient.Height(),
			SWP_SHOWWINDOW);
		m_spRightView->UpdateWindow();
	}else{
		m_spLeftView->SetWindowPos(HWND_BOTTOM,
			rcClient.left + rcFavoriteClient.Width(), rcClient.top,
			rcClient.Width() - rcFavoriteClient.Width(), rcClient.Height(),
			SWP_SHOWWINDOW);
		m_spLeftView->UpdateWindow();
		m_spRightView->ShowWindow(SW_HIDE);
	}

	return 0;
}

LRESULT CFilerWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!m_spCurView) { m_spCurView = m_spLeftView; }
	if(m_spCurView){
		::SetFocus(m_spCurView->GetGridView()->m_hWnd);
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
		m_spGridViewProp,
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
		m_spGridViewProp,
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
		m_spGridViewProp,
		L"FavoritesProperty",
		m_spFavoritesView->GetFavoritesProp());

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

LRESULT CFilerWnd::OnCommandLeftViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandViewOption(m_spLeftView);
}

LRESULT CFilerWnd::OnCommandRightViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandViewOption(m_spRightView);
}


LRESULT CFilerWnd::OnCommandViewOption(std::shared_ptr<CFilerTabGridView>& view)
{
	CRect rc(0, 0, 0, 0);

	auto pPropWnd = new CPropertyWnd<CFilerTabGridView, std::shared_ptr<CGridViewProperty>>(
		m_spGridViewProp,
		L"View",
		view,
		m_spGridViewProp);

	pPropWnd->PropertyChanged.connect([&](const std::wstring& str)->void {
		view->GetGridView()->UpdateAll();
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

