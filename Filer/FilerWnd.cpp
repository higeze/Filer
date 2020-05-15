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
#include "FilerTabGridView.h"
#include "MyFile.h"
#include "ViewProperty.h"
#include "FavoritesProperty.h"
#include "ResourceIDFactory.h"
#include "CellProperty.h"
#include "DriveFolder.h"
#include "ShellFunction.h"
#include "Direct2DWrite.h"
#include "FileIconCache.h"
#include "ThreadSafeDriveFolderManager.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "BindCheckBoxColumn.h"
#include "BindCheckBoxCell.h"
#include "BindSheetCellColumn.h"
#include "BindItemsSheetCell.h"


#include "Task.h"

#ifdef USE_PYTHON_EXTENSION
#include "BoostPythonHelper.h"
#endif

CFilerWnd::CFilerWnd()
	:m_rcWnd(0, 0, 300, 500), 
	m_rcPropWnd(0, 0, 300, 400),
	m_splitterLeft(0),
	m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	m_spFilerGridViewProp(std::make_shared<FilerGridViewProperty>()),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	m_spExeExProp(std::make_shared<ExeExtensionProperty>()),
	m_spLeftView(std::make_shared<CFilerTabGridView>(m_spFilerGridViewProp)),
	m_spRightView(std::make_shared<CFilerTabGridView>(m_spFilerGridViewProp)),
	m_spLeftFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFilerGridViewProp, m_spFavoritesProp)),
	m_spRightFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFilerGridViewProp, m_spFavoritesProp)),
	m_spCurView(m_spLeftView)
#ifdef USE_PYTHON_EXTENSION
	,m_spPyExProp(std::make_shared<CPythonExtensionProperty>())
#endif
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
	.lpszWindowName(L"Filer") 
	.lpszClassName(L"CFilerWnd")
	.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN )
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
	AddMsgHandler(WM_DEVICECHANGE, &CFilerWnd::OnDeviceChange, this);
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
	AddCmdIDHandler(IDM_FAVORITESOPTION,&CFilerWnd::OnCommandFavoritesOption,this);
	AddCmdIDHandler(IDM_EXEEXTENSIONOPTION, &CFilerWnd::OnCommandExeExtensionOption, this);
	AddCmdIDHandler(IDM_LEFTVIEWOPTION, &CFilerWnd::OnCommandLeftViewOption, this);
	AddCmdIDHandler(IDM_RIGHTVIEWOPTION, &CFilerWnd::OnCommandRightViewOption, this);
#ifdef USE_PYTHON_EXTENSION
	AddCmdIDHandler(IDM_PYTHONEXTENSIONOPTION, &CFilerWnd::OnCommandPythonExtensionOption, this);
#endif
}

CFilerWnd::~CFilerWnd()
{

}

HWND CFilerWnd::Create(HWND hWndParent)
{
	HWND hWnd = CWnd::Create(hWndParent, m_rcWnd);
	//WINDOWPLACEMENT wp = { 0 };
	//wp.length = sizeof(WINDOWPLACEMENT);
	//wp.rcNormalPosition = m_rcWnd;
	//::SetWindowPlacement(m_hWnd, &wp);
	return hWnd;
}

LRESULT CFilerWnd::OnCreate(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.rcNormalPosition = m_rcWnd;
	::SetWindowPlacement(m_hWnd, &wp);

	//Konami
	//m_konamiCommander.SetHwnd(m_hWnd);

	//Size
	CRect rcClient = GetClientRect();
	CRect rcLeftFavorites, rcRightFavorites, rcLeftGrid, rcRightGrid;
	d2dw::CDirect2DWrite direct(m_hWnd);
	LONG favoriteWidth = direct.Dips2PixelsX(
		m_spFilerGridViewProp->CellPropPtr->Line->Width * 2 + //def:GridLine=0.5*2, CellLine=0.5*2
		m_spFilerGridViewProp->CellPropPtr->Padding->left + //def:2
		m_spFilerGridViewProp->CellPropPtr->Padding->right + //def:2
		16.f);//icon
	rcLeftFavorites.SetRect(
		rcClient.left, rcClient.top,
		rcClient.left + favoriteWidth, rcClient.bottom);

	if (m_splitterLeft == 0) {//Initial = No serialize

		if (rcClient.Width() >= 800) {
			LONG viewWidth = (rcClient.Width() - 2 * favoriteWidth - kSplitterWidth) / 2;
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top,
				rcClient.left + favoriteWidth + viewWidth,
				rcClient.bottom);

			rcRightFavorites.SetRect(
				rcClient.left + favoriteWidth + viewWidth + kSplitterWidth,
				rcClient.top,
				rcClient.left + favoriteWidth + viewWidth + kSplitterWidth + favoriteWidth,
				rcClient.bottom);

			rcRightGrid.SetRect(
				rcClient.left + favoriteWidth + viewWidth + kSplitterWidth + favoriteWidth,
				rcClient.top,
				rcClient.right, 
				rcClient.bottom);
			m_splitterLeft = favoriteWidth + viewWidth;
		} else {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth, 
				rcClient.top,
				rcClient.right,
				rcClient.bottom);
			m_splitterLeft = rcClient.right;
		}
	} else {
		if (rcClient.Width() >= m_splitterLeft + kSplitterWidth) {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top,
				m_splitterLeft,
				rcClient.bottom);

			rcRightFavorites.SetRect(
				m_splitterLeft + kSplitterWidth,
				rcClient.top,
				m_splitterLeft + kSplitterWidth + favoriteWidth,
				rcClient.bottom);

			rcRightGrid.SetRect(
				m_splitterLeft + kSplitterWidth + favoriteWidth,
				rcClient.top,
				rcClient.right,// - (m_splitterLeft + kSplitterWidth + favoriteWidth), 
				rcClient.bottom);
		} else {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top,
				rcClient.right, 
				rcClient.bottom);
		}
	}

	//CFavoritesGridView
	auto createFavoritesView = [this](std::shared_ptr<CFavoritesGridView>& spFavoritesView, std::shared_ptr<CFilerTabGridView>& spView, unsigned short id, CRect& rc)->void {
		spFavoritesView->CreateWindowExArgument()
			.dwStyle(spFavoritesView->CreateWindowExArgument().dwStyle() | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | (rc.operator bool() ? WS_VISIBLE : 0))
			.hMenu((HMENU)id);

		spFavoritesView->FileChosen = [&](std::shared_ptr<CShellFile>& spFile)->void {
			spView->GetGridView()->Open(spFile);
			spView->GetGridView()->SetFocus();
			spView->GetGridView()->SubmitUpdate();
		};
		
		spFavoritesView->Create(m_hWnd, rc);
	};
	createFavoritesView(m_spLeftFavoritesView, m_spLeftView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"LeftFavoritesGridView"), rcLeftFavorites);
	createFavoritesView(m_spRightFavoritesView, m_spRightView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"RightFavoritesGridView"), rcRightFavorites);

	//CFilerTabGridView
	auto createFilerTabGridView = [this](std::shared_ptr<CFilerTabGridView>& spView, unsigned short id, CRect& rc)->void {
		spView->SetParentWnd(this);
		spView->CreateWindowExArgument()
			.dwStyle(spView->CreateWindowExArgument().dwStyle() | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | (rc.operator bool() ? WS_VISIBLE : 0))
			.hMenu((HMENU)id);
		spView->Create(m_hWnd, rc);
		spView->SubclassWindow(spView->m_hWnd);
		BOOL dummy = FALSE;
		spView->OnCreate(WM_CREATE, NULL, NULL, dummy);
		//Capture KeyDown Msg in FilerView
		auto funKeyDown = spView->GetGridView()->GetMsgHandler(WM_KEYDOWN);
		if (funKeyDown) {
			FunMsg newFun = [this, funKeyDown](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
				LRESULT lr = OnKeyDown(uMsg, wParam, lParam, bHandled);
				if (bHandled) {
					return lr;
				} else {
					return funKeyDown(uMsg, wParam, lParam, bHandled);
				}
			};
			spView->GetGridView()->ReplaceMsgHandler(WM_KEYDOWN, newFun);
		}

		auto funSetFocus = spView->GetGridView()->GetMsgHandler(WM_SETFOCUS);
		if (funSetFocus){
			std::weak_ptr<CFilerTabGridView> wp(spView);
			FunMsg newFun = [this, wp, funSetFocus](UINT uMsg, LPARAM lParam, WPARAM wParam, BOOL& bHandled)->LRESULT {
				if (auto sp = wp.lock()) {
					m_spCurView = sp;
					m_spCurView->GetGridView()->InvalidateRect(NULL, FALSE);
				}
				return funSetFocus(uMsg, lParam, wParam, bHandled);
			};
			spView->GetGridView()->ReplaceMsgHandler(WM_SETFOCUS, newFun);
		}
	};


	createFilerTabGridView(m_spLeftView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"LeftFilerGridView"), rcLeftGrid);
	createFilerTabGridView(m_spRightView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"RightFilerGridView"), rcRightGrid);

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

#ifdef USE_PYTHON_EXTENSION
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
#endif
			menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);

			for (auto& ex : m_spExeExProp->ExeExtensions) {

				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_TYPE | MIIM_ID;
				mii.fType = MFT_STRING;
				mii.fState = MFS_ENABLED;
				mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, std::get<ExeExtension>(ex).Name);
				mii.dwTypeData = (LPWSTR)std::get<ExeExtension>(ex).Name.c_str();
				menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
			}

		};
		spFilerView->ExecCustomContextMenu = [&](int idCmd, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl)->bool {
			if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToFavoritesFromItem")) {
				for (auto& pIdl : vpIdl) {
					STRRET strret;
					psf->GetDisplayNameOf(pIdl, SHGDN_FORPARSING, &strret);
					std::wstring path = shell::strret2wstring(strret, pIdl);
					GetFavoritesPropPtr()->GetFavorites().notify_push_back(std::make_tuple(std::make_shared<CFavorite>(path, L"")));
					//m_spLeftFavoritesView->PushRow(std::make_shared<CBindRow<std::shared_ptr<CShellFile>>>(m_spLeftFavoritesView.get()));
					//m_spRightFavoritesView->PushRow(std::make_shared<CBindRow<std::shared_ptr<CShellFile>>>(m_spRightFavoritesView.get()));
				}
				m_spLeftFavoritesView->SubmitUpdate();
				m_spRightFavoritesView->SubmitUpdate();
				return true;
			} else {
				auto iter = std::find_if(m_spExeExProp->ExeExtensions.begin(), m_spExeExProp->ExeExtensions.end(),
					[idCmd](const auto& exeex)->bool {
					return CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, std::get<ExeExtension>(exeex).Name) == idCmd;
				});
				if (iter != m_spExeExProp->ExeExtensions.end()) {
					try {
						auto& exeExtension = std::get<ExeExtension>(*iter);
						std::vector<std::wstring> filePaths;
						for (auto& pIdl : vpIdl) {
							STRRET strret;
							psf->GetDisplayNameOf(pIdl, SHGDN_FORPARSING, &strret);
							filePaths.emplace_back(L"\"" + shell::strret2wstring(strret, pIdl) + L"\"");
						}

						std::wstring fileMultiPath = boost::join(filePaths, L" ");

						std::wstring parameter = exeExtension.Parameter;

						std::wstring singlePathUnQuo = filePaths[0];
						::PathUnquoteSpaces(::GetBuffer(singlePathUnQuo, MAX_PATH));
						::ReleaseBuffer(singlePathUnQuo);

						std::wstring singlePathWoExtUnQuo = singlePathUnQuo;
						::PathRemoveExtension(::GetBuffer(singlePathWoExtUnQuo, MAX_PATH));
						::ReleaseBuffer(singlePathWoExtUnQuo);

						boost::algorithm::replace_all(parameter, L"%SinglePath%", filePaths[0]);
						boost::algorithm::replace_all(parameter, L"%SinglePathUnQuo%", singlePathUnQuo);
						boost::algorithm::replace_all(parameter, L"%MultiPath%", fileMultiPath);
						boost::algorithm::replace_all(parameter, L"%DirectoryUnQuo%", ::PathFindDirectory(singlePathUnQuo.c_str()));
						boost::algorithm::replace_all(parameter, L"%SinglePathWoExtUnQuo%", singlePathWoExtUnQuo );	
							
						std::wstring cmdline = L"\"" + exeExtension.Path + L"\" " + parameter;

						HANDLE hRead, hWrite;
						SECURITY_ATTRIBUTES sa = { 0 };
						sa.nLength = sizeof(sa);
						sa.lpSecurityDescriptor = 0;
						sa.bInheritHandle = TRUE;
						if (!::CreatePipe(&hRead, &hWrite, &sa, 0)) {
							return false;
						}
						do {
							STARTUPINFO si = { 0 };
							si.cb = sizeof(si);
							si.dwFlags = STARTF_USESTDHANDLES;
							si.wShowWindow = SW_HIDE;
							si.hStdOutput = hWrite;
							si.hStdError = hWrite;

							PROCESS_INFORMATION pi = { 0 };
							DWORD len = 0;
							SPDLOG_INFO("CreateProcess: {}", wstr2str(cmdline));

							if (!::CreateProcess(NULL, const_cast<LPWSTR>(cmdline.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) break;
							if (!::WaitForInputIdle(pi.hProcess, INFINITE) != 0)break;
							if (::WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)break;

							DWORD exitCode = 0;
							if (!::GetExitCodeProcess(pi.hProcess, &exitCode))break;
							SPDLOG_INFO("ExitCode: {}", exitCode);

							::CloseHandle(pi.hThread);
							::CloseHandle(pi.hProcess);

							if (!::PeekNamedPipe(hRead, NULL, 0, NULL, &len, NULL))break;
							std::string buff;
							if (len > 0 && ReadFile(hRead, (LPVOID)::GetBuffer(buff, len), len, &len, NULL)) {
								::ReleaseBuffer(buff);
								SPDLOG_INFO("Output: {}", buff);
							}

						} while (0);

						::CloseHandle(hRead);
						::CloseHandle(hWrite);
					} catch (...) {
						throw;
					}
				return true;
			} else {
					return false;
				}

#ifdef USE_PYTHON_EXTENSION
				auto iter = std::find_if(m_spPyExProp->PythonExtensions.begin(), m_spPyExProp->PythonExtensions.end(),
					[idCmd](const auto& pyex)->bool {
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
#else
				return false;
#endif
			}
		};
	};

	applyCustomContextMenu(m_spLeftView->GetGridView());
	applyCustomContextMenu(m_spRightView->GetGridView());

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
			std::shared_ptr<CFilerTabGridView> spOtherView = m_spCurView == m_spLeftView ? m_spRightView : m_spLeftView;
			m_spCurView->GetGridView()->CopySelectedFilesTo(spOtherView->GetGridView()->GetFolder()->GetAbsoluteIdl());
			bHandled = FALSE;
		}
		break;
	case VK_F6:
		{
			std::shared_ptr<CFilerTabGridView> spOtherView = m_spCurView == m_spLeftView ? m_spRightView : m_spLeftView;
			m_spCurView->GetGridView()->MoveSelectedFilesTo(spOtherView->GetGridView()->GetFolder()->GetAbsoluteIdl());
			bHandled = FALSE;
		}
		break;
	case VK_F9:
		{
			int okcancel = ::MessageBox(m_spCurView->GetGridView()->m_hWnd, L"Incremental Copy?", L"Incremental Copy?", MB_OKCANCEL);
			if (okcancel == IDOK) {
				std::shared_ptr<CFilerTabGridView> spOtherView = m_spCurView == m_spLeftView ? m_spRightView : m_spLeftView;
				m_spCurView->GetGridView()->CopyIncrementalSelectedFilesTo(spOtherView->GetGridView()->GetFolder()->GetAbsoluteIdl());
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
		m_ptBeginClient = pt;
		SetCapture();
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	}
	return 0;
}

LRESULT CFilerWnd::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	if (m_isSizing)
	{
		m_ptBeginClient.SetPoint(0, 0);
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
		m_splitterLeft += pt.x - m_ptBeginClient.x;
		m_ptBeginClient = pt;
		PostMessage(WM_SIZE, 0, 0);
	}
	//m_konamiCommander.OnMouseMove(uMsg, wParam, lParam, bHandled);
	return 0;
}

LRESULT CFilerWnd::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CRect rcClient = GetClientRect();

	//Favorites
	CRect rcFavoriteClient = m_spLeftFavoritesView->GetDirectPtr()->Dips2Pixels(m_spLeftFavoritesView->GetRect());
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

LRESULT CFilerWnd::OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_spLeftView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	//m_spRightView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	CDriveFolderManager::GetInstance()->Update();
	m_spLeftFavoritesView->Reload();
	m_spRightFavoritesView->Reload();
	m_spLeftView->GetGridView()->Reload();
	m_spRightView->GetGridView()->Reload();

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

LRESULT CFilerWnd::OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	return OnCommandOption<CFavoritesProperty>(L"Favorites Property", m_spFavoritesProp,
		[this](const std::wstring& str)->void {
		m_spLeftFavoritesView->Reload();
		m_spRightFavoritesView->Reload();
		InvalidateRect(NULL, FALSE);
		SerializeProperty(this);
	});
}

LRESULT CFilerWnd::OnCommandExeExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandOption<ExeExtensionProperty>(L"Exe extension Property", m_spExeExProp,
		[this](const std::wstring& str)->void {
		SerializeProperty(this);
	});
}

LRESULT CFilerWnd::OnCommandLeftViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//TODOTODO
	//auto pBindWnd = new CBindGridView<MainTask>(std::static_pointer_cast<GridViewProperty>(m_spFilerGridViewProp));

	//pBindWnd->RegisterClassExArgument()
	//	.lpszClassName(L"CBindWnd")
	//	.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
	//	.hCursor(::LoadCursor(NULL, IDC_ARROW))
	//	.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	//pBindWnd->CreateWindowExArgument()
	//	.lpszClassName(_T("CBindWnd"))
	//	.lpszWindowName(L"TODO")
	//	.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
	//	.dwExStyle(WS_EX_ACCEPTFILES)
	//	.hMenu(NULL);


	//pBindWnd->SetIsDeleteOnFinalMessage(true);

	////Columns
	//pBindWnd->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(pBindWnd));
	//pBindWnd->PushColumns(
	//	pBindWnd->GetHeaderColumnPtr(),
	//	std::make_shared<CBindCheckBoxColumn<MainTask>>(
	//		pBindWnd,
	//		L"Done",
	//		[](const std::tuple<MainTask>& tk)->CheckBoxState {return std::get<MainTask>(tk).Done?CheckBoxState::True:CheckBoxState::False; },
	//		[](std::tuple<MainTask>& tk, const CheckBoxState& state)->void {std::get<MainTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
	//	std::make_shared<CBindTextColumn<MainTask>>(
	//		pBindWnd,
	//		L"Name",
	//		[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Name; },
	//		[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Name = str; }),
	//	std::make_shared<CBindTextColumn<MainTask>>(
	//		pBindWnd,
	//		L"Memo",
	//		[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Memo; },
	//		[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Memo = str; }),
	//	std::make_shared<CBindSheetCellColumn< MainTask, SubTask>>(
	//		pBindWnd,
	//		L"Sub Task",
	//		[](std::tuple<MainTask>& tk)->observable_vector<SubTask>& {return std::get<MainTask>(tk).SubTasks; },
	//		[](CBindItemsSheetCell<MainTask, SubTask>* pCell)->void 
	//		{
	//			pCell->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(pCell));
	//			pCell->PushColumns(
	//				pCell->GetHeaderColumnPtr(),
	//				std::make_shared<CBindCheckBoxColumn<SubTask>>(
	//					pCell,
	//					L"Done",
	//					[](const std::tuple<SubTask>& tk)->CheckBoxState {return std::get<SubTask>(tk).Done ? CheckBoxState::True : CheckBoxState::False; },
	//					[](std::tuple<SubTask>& tk, const CheckBoxState& state)->void {std::get<SubTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
	//				std::make_shared<CBindTextColumn<SubTask>>(
	//					pCell,
	//					L"Name",
	//					[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Name; },
	//					[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Name = str; }),
	//				std::make_shared<CBindTextColumn<SubTask>>(
	//					pCell,
	//					L"Memo",
	//					[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Memo; },
	//					[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Memo = str; })
	//			);
	//			pCell->SetFrozenCount<ColTag>(1);

	//			pCell->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(pCell));
	//			pCell->InsertRow(0, pCell->GetNameHeaderRowPtr());
	//			pCell->SetFrozenCount<RowTag>(1);
	//		},
	//		arg<"maxwidth"_s>() = FLT_MAX)
	//	);
	//pBindWnd->SetFrozenCount<ColTag>(1);

	////Rows
	//pBindWnd->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(pBindWnd));
	//pBindWnd->SetFilterRowPtr(std::make_shared<CRow>(pBindWnd));

	//pBindWnd->PushRows(
	//	pBindWnd->GetNameHeaderRowPtr(),
	//	pBindWnd->GetFilterRowPtr());

	//pBindWnd->SetFrozenCount<RowTag>(2);

	//pBindWnd->GetItemsSource().notify_push_back(MainTask{ true, L"Test", L"mon" });
	//pBindWnd->GetItemsSource().notify_push_back(MainTask{ false, L"PVA", L"Tue" });
	//pBindWnd->GetItemsSource().notify_push_back(MainTask{ true, L"NAME", L"Run" });
	//std::get<MainTask>(pBindWnd->GetItemsSource()[2]).SubTasks.notify_push_back(SubTask{ true, L"SUB", L"TODO" });
	//std::get<MainTask>(pBindWnd->GetItemsSource()[2]).SubTasks.notify_push_back(SubTask{ true, L"SUB2", L"TET" });

	//HWND hWnd = NULL;
	//if ((GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
	//	hWnd = m_hWnd;
	//} else {
	//	hWnd = GetAncestorByStyle(WS_OVERLAPPEDWINDOW);
	//}

	//pBindWnd->CreateOnCenterOfParent(NULL, CSize(400, 600));
	//pBindWnd->ShowWindow(SW_SHOW);
	//pBindWnd->UpdateWindow();
	return 0;
}

LRESULT CFilerWnd::OnCommandRightViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//return OnCommandOption<CFilerTabGridView, std::shared_ptr<FilerGridViewProperty>>(
	//	L"Right View", m_spRightView,
	//	[this](const std::wstring& str)->void {
	//	m_spLeftFavoritesView->UpdateAll();
	//	m_spRightFavoritesView->UpdateAll();
	//	SerializeProperty(this);}, 
	//	m_spFilerGridViewProp);
	return 0;
}

#ifdef USE_PYTHON_EXTENSION
LRESULT CFilerWnd::OnCommandPythonExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCommandOption<CPythonExtensionProperty>(L"Python extension Property", m_spPyExProp,
		[this](const std::wstring& str)->void {
		SerializeProperty(this);
	});
}
#endif


