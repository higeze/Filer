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
#include "MyFile.h"
#include "BoostPythonHelper.h"
#include "ViewProperty.h"
#include "FavoritesProperty.h"
#include "ResourceIDFactory.h"
#include "CellProperty.h"

CFilerWnd::CFilerWnd()
	:m_rcWnd(0, 0, 300, 500), 
	m_splitterLeft(0),
	m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	m_spGridViewProp(std::make_shared<CGridViewProperty>()),
	m_spFilerGridViewProp(std::make_shared<FilerGridViewProperty>()),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	m_spPyExProp(std::make_shared<CPythonExtensionProperty>()),
	m_spLeftView(std::make_shared<CFilerTabGridView>(m_spGridViewProp, m_spFilerGridViewProp)),
	m_spRightView(std::make_shared<CFilerTabGridView>(m_spGridViewProp, m_spFilerGridViewProp)),
	m_spLeftFavoritesView(std::make_shared<CFavoritesGridView>(m_spGridViewProp, m_spFavoritesProp)),
	m_spRightFavoritesView(std::make_shared<CFavoritesGridView>(m_spGridViewProp, m_spFavoritesProp)),
	m_spCurView(m_spLeftView)
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
	//AddMsgHandler(WM_SETCURSOR, &CKonamiCommander::OnSetCursor, &m_konamiCommander);
	//AddMsgHandler(WM_PAINT, &CKonamiCommander::OnPaint, &m_konamiCommander);

	AddCmdIDHandler(IDM_SAVE,&CFilerWnd::OnCommandSave,this);
	AddCmdIDHandler(IDM_EXIT, &CFilerWnd::OnCommandExit, this);

	//AddCmdIDHandler(IDM_EDIT_COPY,&CMultiLineListView::OnCmdEditCopy,(CMultiLineListView*)m_upList.get());
	//AddCmdIDHandler(IDM_EDIT_SELECTALL,&CMultiLineListView::OnCmdEditSelectAll,(CMultiLineListView*)m_upList.get());
	AddCmdIDHandler(IDM_APPLICATIONOPTION, &CFilerWnd::OnCommandApplicationOption, this);
	AddCmdIDHandler(IDM_FILERGRIDVIEWOPTION, &CFilerWnd::OnCommandFilerGridViewOption, this);
	AddCmdIDHandler(IDM_GRIDVIEWOPTION,&CFilerWnd::OnCommandGridViewOption,this);
	AddCmdIDHandler(IDM_FAVORITESOPTION,&CFilerWnd::OnCommandFavoritesOption,this);
	AddCmdIDHandler(IDM_PYTHONEXTENSIONOPTION, &CFilerWnd::OnCommandPythonExtensionOption, this);

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
	//Konami
	//m_konamiCommander.SetHwnd(m_hWnd);

	//CFavoritesGridView
	auto createFavoritesView = [this](std::shared_ptr<CFavoritesGridView>& spFavoritesView, std::shared_ptr<CFilerTabGridView>& spView, unsigned short id)->void {
		spFavoritesView->CreateWindowExArgument()
			.dwStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE)
			.hMenu((HMENU)id);
		spFavoritesView->FileChosen.connect([&](std::shared_ptr<CShellFile>& spFile)->void {
			spView->GetGridView()->Open(spFile);
		});
		spFavoritesView->Create(m_hWnd);
	};
	createFavoritesView(m_spLeftFavoritesView, m_spLeftView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"LeftFavoritesGridView"));
	createFavoritesView(m_spRightFavoritesView, m_spRightView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"RightFavoritesGridView"));

	//CFilerTabGridView
	auto createFilerTabGridView = [this](std::shared_ptr<CFilerTabGridView>& spView, unsigned short id)->void {
		spView->SetParentWnd(this);
		m_spLeftView->CreateWindowExArgument().hMenu((HMENU)id);
		spView->Create(m_hWnd);
		spView->SubclassWindow(spView->m_hWnd);
		BOOL dummy = FALSE;
		spView->OnCreate(WM_CREATE, NULL, NULL, dummy);
		//Capture KeyDown Msg in FilerView
		auto fun = spView->GetGridView()->GetMsgHandler(WM_KEYDOWN);
		if (fun) {
			FunMsg newFun = [this, fun](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
				LRESULT lr = OnKeyDown(uMsg, wParam, lParam, bHandled);
				if (bHandled) {
					return lr;
				} else {
					return fun(uMsg, wParam, lParam, bHandled);
				}
			};
			spView->GetGridView()->ReplaceMsgHandler(WM_KEYDOWN, newFun);
		}

		spView->GetGridView()->AddMsgHandler(WM_SETFOCUS,
			[&](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT {
			m_spCurView = spView;
			m_spCurView->GetGridView()->InvalidateRect(NULL, FALSE);
			return 0;
		});
	};

	createFilerTabGridView(m_spLeftView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"LeftFilerGridView"));
	createFilerTabGridView(m_spRightView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"RightFavoritesGridView"));

	auto applyCustomContextMenu = [this](std::shared_ptr<CFilerGridView> spFilerView)->void {

		spFilerView->AddCustomContextMenu = [&](CMenu& menu) {
			menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);
			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_TYPE | MIIM_ID;
			mii.fType = MFT_STRING;
			mii.fState = MFS_ENABLED;
			mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToFavoritesFromItem");
			mii.dwTypeData = L"Add to favorite";
			menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

			menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);

			for (auto& pyex : m_spPyExProp->PythonExtensions) {
				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_TYPE | MIIM_ID;
				mii.fType = MFT_STRING;
				mii.fState = MFS_ENABLED;
				mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, pyex.Name);
				mii.dwTypeData = (LPWSTR)pyex.Name.c_str();
				menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
			}
		};

		spFilerView->ExecCustomContextMenu = [&](int idCmd, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl)->bool {
			if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToFavoritesFromItem")) {
				for (auto& pIdl : vpIdl) {
					STRRET strret;
					psf->GetDisplayNameOf(pIdl, SHGDN_FORPARSING, &strret);
					std::wstring path = STRRET2WSTR(strret, pIdl);
					GetFavoritesPropPtr()->GetFavorites()->push_back(CFavorite(path, L""));
					m_spLeftFavoritesView->InsertRow(CRow::kMaxIndex, std::make_shared<CFavoriteRow>(m_spLeftFavoritesView.get(), GetFavoritesPropPtr()->GetFavorites()->size() - 1));
					m_spRightFavoritesView->InsertRow(CRow::kMaxIndex, std::make_shared<CFavoriteRow>(m_spRightFavoritesView.get(), GetFavoritesPropPtr()->GetFavorites()->size() - 1));
				}
				m_spLeftFavoritesView->SubmitUpdate();
				m_spRightFavoritesView->SubmitUpdate();
				return true;
			}else{
				auto iter = std::find_if(m_spPyExProp->PythonExtensions.begin(), m_spPyExProp->PythonExtensions.end(),
					[idCmd](const auto& pyex)->bool
					{
						return CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, pyex.Name) == idCmd;
					});
				if (iter != m_spPyExProp->PythonExtensions.end()) {
					try {

						if (!m_spPyExProp->PythonHome.empty()) {
							Py_SetPythonHome(m_spPyExProp->PythonHome.c_str());
						}
						Py_Initialize();

						boost::python::list li;
						for (auto& pIdl : vpIdl) {
							STRRET strret;
							psf->GetDisplayNameOf(pIdl, SHGDN_FORPARSING, &strret);
							std::wstring path = STRRET2WSTR(strret, pIdl);

							li.append(path);
						}

						boost::python::object global_ns = boost::python::import("__main__").attr("__dict__");

						std::string script = CFile::ReadAllString<char>(iter->ScriptPath);
						boost::python::exec(script.c_str(), global_ns, global_ns);
						boost::python::object pythonFun = global_ns[iter->FunctionName];

						pythonFun(li);
					} catch (boost::python::error_already_set&) {
						MessageBoxA(m_hWnd, parse_python_exception().c_str(), "Python error", MB_OK);
					} catch (...) {
						MessageBoxA(m_hWnd, "Error on running Python", "Python error", MB_OK);
					}
					return true;
				} else {
					return false;
				}
			}
		};
	};

	applyCustomContextMenu(m_spLeftView->GetGridView());
	applyCustomContextMenu(m_spRightView->GetGridView());

	//Size
	CRect rcClient = GetClientRect();
	if (m_splitterLeft == 0) {
		//Initialize window position
		CRect rcFavoriteClient = m_spLeftFavoritesView->GetDirect()->Dips2Pixels(m_spLeftFavoritesView->GetRect());
		m_spLeftFavoritesView->SetWindowPos(HWND_BOTTOM,
			rcClient.left, rcClient.top,
			rcFavoriteClient.Width(), rcClient.Height(),
			SWP_SHOWWINDOW);

		if (rcClient.Width() >= 800) {
			LONG viewWidth = (rcClient.Width() - 2 * rcFavoriteClient.Width() - kSplitterWidth) / 2;
			m_spLeftView->SetWindowPos(HWND_BOTTOM,
				rcClient.left + rcFavoriteClient.Width(),
				rcClient.top,
				viewWidth, rcClient.Height(),
				SWP_SHOWWINDOW);

			m_spRightFavoritesView->SetWindowPos(HWND_BOTTOM,
				rcClient.left + rcFavoriteClient.Width() + kSplitterWidth + viewWidth,
				rcClient.top,
				rcFavoriteClient.Width(), rcClient.Height(),
				SWP_SHOWWINDOW);

			m_spRightView->SetWindowPos(HWND_BOTTOM,
				rcClient.left + rcFavoriteClient.Width() + kSplitterWidth + viewWidth + rcFavoriteClient.Width(),
				rcClient.top,
				(rcClient.Width() - 2 * rcFavoriteClient.Width() - kSplitterWidth) / 2, rcClient.Height(),
				SWP_SHOWWINDOW);
			m_splitterLeft = rcFavoriteClient.Width() + viewWidth;
		} else {
			m_spLeftView->SetWindowPos(HWND_BOTTOM,
				rcClient.left + rcFavoriteClient.Width(), rcClient.top,
				rcClient.Width() - rcFavoriteClient.Width(), rcClient.Height(),
				SWP_SHOWWINDOW);
			m_spRightFavoritesView->ShowWindow(SW_HIDE);
			m_spRightView->ShowWindow(SW_HIDE);
			m_splitterLeft = rcClient.right;
		}
		//CRect rcLeftView(m_spLeftView->GetClientRect());
		//m_leftSplit = ScreenToClientRect(m_spLeftView->GetWindowRect());
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
	case 'S':
		{
			if (::GetAsyncKeyState(VK_CONTROL)) {
				BOOL dummy;
				OnCommandSave(0, 0, 0, dummy);
				bHandled = TRUE;
			}
		}
		break;
	case VK_F4:
		{
			if (::GetAsyncKeyState(VK_MENU)) {
				BOOL dummy;
				OnCommandExit(0, 0, 0, dummy);
				bHandled = TRUE;
			}
		}
		break;
	case VK_F5:
		{
			int okcancel = ::MessageBox(m_hWnd, L"Copy?", L"Copy?", MB_OKCANCEL);
			if (okcancel == IDOK) {
				std::shared_ptr<CFilerTabGridView> spOtherView = m_spCurView == m_spLeftView ? m_spRightView : m_spLeftView;
				CComPtr<IShellItem2> pDestShellItem;

				HRESULT hr = ::SHCreateItemFromIDList(spOtherView->GetGridView()->GetFolder()->GetAbsoluteIdl().ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem));
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

				HRESULT hr = ::SHCreateItemFromIDList(spOtherView->GetGridView()->GetFolder()->GetAbsoluteIdl().ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem));
				if (FAILED(hr)) { break; }
				m_spCurView->GetGridView()->MoveTo(pDestShellItem);
			}
			bHandled = TRUE;
	}
		break;
	default:
		break;
	}
	//m_konamiCommander.OnKeyDown(uMsg, wParam, lParam, bHandled);
	bHandled = FALSE;
	return 0;
}

LRESULT CFilerWnd::OnClose(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{	
	WINDOWPLACEMENT wp={0};
	wp.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	m_rcWnd=CRect(wp.rcNormalPosition);
	
	m_spLeftFavoritesView->DestroyWindow();
	m_spRightFavoritesView->DestroyWindow();
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
		m_splitterLeft += pt.x - m_ptStart.x;
		m_ptStart = pt;
		PostMessage(WM_SIZE, 0, 0);
	}
	//m_konamiCommander.OnMouseMove(uMsg, wParam, lParam, bHandled);
	return 0;
}


LRESULT CFilerWnd::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CRect rcClient = GetClientRect();

	//Favorites
	CRect rcFavoriteClient = m_spLeftFavoritesView->GetDirect()->Dips2Pixels(m_spLeftFavoritesView->GetRect());
    m_spLeftFavoritesView->SetWindowPos(HWND_BOTTOM,
        rcClient.left , rcClient.top, 
        rcFavoriteClient.Width(), rcClient.Height(),
        SWP_SHOWWINDOW);

	if (rcClient.Width() >= m_splitterLeft + kSplitterWidth) {
		m_spLeftView->SetWindowPos(HWND_BOTTOM,
			rcClient.left + rcFavoriteClient.Width(), 
			rcClient.top,
			m_splitterLeft-(rcClient.left + rcFavoriteClient.Width()), rcClient.Height(),
			SWP_SHOWWINDOW);
		m_spLeftView->UpdateWindow();

		m_spRightFavoritesView->SetWindowPos(HWND_BOTTOM,
			m_splitterLeft + kSplitterWidth,
			rcClient.top,
			rcFavoriteClient.Width(), rcClient.Height(),
			SWP_SHOWWINDOW);

		m_spRightView->SetWindowPos(HWND_BOTTOM,
			m_splitterLeft + kSplitterWidth + rcFavoriteClient.Width(),
			rcClient.top,
			rcClient.right - (m_splitterLeft + kSplitterWidth + rcFavoriteClient.Width()), rcClient.Height(),
			SWP_SHOWWINDOW);
		m_spRightView->UpdateWindow();
	}else{
		m_spLeftView->SetWindowPos(HWND_BOTTOM,
			rcClient.left + rcFavoriteClient.Width(),
			rcClient.top,
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


LRESULT CFilerWnd::OnCommandSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SerializeProperty(this);
	return 0;
}

LRESULT CFilerWnd::OnCommandExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BOOL dummy = FALSE;
	OnClose(WM_CLOSE, NULL, NULL, dummy);
	return 0;
}


LRESULT CFilerWnd::OnCommandApplicationOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandOption<CApplicationProperty>(L"Application Property", m_spApplicationProp,
		[this](const std::wstring& str)->void {
			SerializeProperty(this);
	});
}

LRESULT CFilerWnd::OnCommandFilerGridViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandOption<FilerGridViewProperty>(L"FilerGridView Property", m_spFilerGridViewProp,
		[this](const std::wstring& str)->void {
		m_spLeftView->GetGridView()->UpdateAll();
		m_spRightView->GetGridView()->UpdateAll();
		SerializeProperty(this);
	});
}

LRESULT CFilerWnd::OnCommandGridViewOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	return OnCommandOption<CGridViewProperty>(L"GridView Property", m_spGridViewProp,
		[this](const std::wstring& str)->void {
		m_spLeftView->GetGridView()->UpdateAll();
		m_spRightView->GetGridView()->UpdateAll();
		m_spLeftFavoritesView->UpdateAll();
		m_spRightFavoritesView->UpdateAll();
		SerializeProperty(this);
	});
}

LRESULT CFilerWnd::OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	return OnCommandOption<CFavoritesProperty>(L"Favorites Property", m_spFavoritesProp,
		[this](const std::wstring& str)->void {
		m_spLeftFavoritesView->UpdateAll();
		m_spRightFavoritesView->UpdateAll();
		SerializeProperty(this);
	});
}

LRESULT CFilerWnd::OnCommandPythonExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandOption<CPythonExtensionProperty>(L"Python extension Property", m_spPyExProp,
		[this](const std::wstring& str)->void {
		SerializeProperty(this);
	});
}


LRESULT CFilerWnd::OnCommandLeftViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandOption<CFilerTabGridView, std::shared_ptr<CGridViewProperty>, std::shared_ptr<FilerGridViewProperty>>(
		L"Left View", m_spLeftView,
		[this](const std::wstring& str)->void {
		m_spLeftFavoritesView->UpdateAll();
		m_spRightFavoritesView->UpdateAll();
		SerializeProperty(this);
	}, m_spGridViewProp, m_spFilerGridViewProp);
}

LRESULT CFilerWnd::OnCommandRightViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandOption<CFilerTabGridView, std::shared_ptr<CGridViewProperty>, std::shared_ptr<FilerGridViewProperty>>(
		L"Right View", m_spRightView,
		[this](const std::wstring& str)->void {
		m_spLeftFavoritesView->UpdateAll();
		m_spRightFavoritesView->UpdateAll();
		SerializeProperty(this);
	}, m_spGridViewProp, m_spFilerGridViewProp);
}


