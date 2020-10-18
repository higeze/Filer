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

#include "ToDoGridView.h"
#include "D2DWWindow.h"
#include "MouseStateMachine.h"
#include <Dbt.h>

#ifdef USE_PYTHON_EXTENSION
#include "BoostPythonHelper.h"
#endif

CFilerWnd::CFilerWnd()
	:CD2DWWindow(),
	m_rcWnd(0, 0, 1000, 600), 
	m_rcPropWnd(0, 0, 300, 400),
	m_splitterLeft(0),
	m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	m_spFilerGridViewProp(std::make_shared<FilerGridViewProperty>()),
	m_spTextEditorProp(std::make_shared<TextEditorProperty>()),
	m_spTabControlProp(std::make_shared<TabControlProperty>()),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	m_spExeExProp(std::make_shared<ExeExtensionProperty>()),
	m_spLeftView(std::make_shared<CFilerTabGridView>(this, std::make_shared<TabControlProperty>(), m_spFilerGridViewProp, m_spTextEditorProp)),
	m_spRightView(std::make_shared<CFilerTabGridView>(this, std::make_shared<TabControlProperty>(), m_spFilerGridViewProp, m_spTextEditorProp)),
	m_spLeftFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFilerGridViewProp, m_spFavoritesProp)),
	m_spRightFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFilerGridViewProp, m_spFavoritesProp)),
	m_spStatusBar(std::make_shared<CStatusBar>(this, std::make_shared<StatusBarProperty>())),
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
		.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)
		.x(m_rcWnd.left)
		.y(m_rcWnd.top)
		.nWidth(m_rcWnd.Width())
		.nHeight(m_rcWnd.Height())
		;// .hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"FilerWnd"));


	AddMsgHandler(WM_DESTROY, &CFilerWnd::OnDestroy, this);
	AddMsgHandler(WM_DEVICECHANGE, &CFilerWnd::OnDeviceChange, this);

	m_commandMap.emplace(IDM_SAVE, std::bind(&CFilerWnd::OnCommandSave, this, phs::_1));
	m_commandMap.emplace(IDM_EXIT, std::bind(&CFilerWnd::OnCommandExit, this, phs::_1));
	m_commandMap.emplace(IDM_APPLICATIONOPTION, std::bind(&CFilerWnd::OnCommandApplicationOption, this, phs::_1));
	m_commandMap.emplace(IDM_FILERGRIDVIEWOPTION, std::bind(&CFilerWnd::OnCommandFilerGridViewOption, this, phs::_1));
	m_commandMap.emplace(IDM_TEXTOPTION, std::bind(&CFilerWnd::OnCommandTextOption, this, phs::_1));
	m_commandMap.emplace(IDM_FAVORITESOPTION, std::bind(&CFilerWnd::OnCommandFavoritesOption,this, phs::_1));
	m_commandMap.emplace(IDM_EXEEXTENSIONOPTION, std::bind(&CFilerWnd::OnCommandExeExtensionOption, this, phs::_1));

#ifdef USE_PYTHON_EXTENSION
	m_commandMap.emplace(IDM_PYTHONEXTENSIONOPTION, std::bind(&CFilerWnd::OnCommandPythonExtensionOption, this, phs::_1));
#endif

	//CFavoritesGridView
	auto setUpFavoritesView = [this](std::shared_ptr<CFavoritesGridView>& spFavoritesGridView, std::weak_ptr<CFilerTabGridView>& wpView, unsigned short id)->void {
		spFavoritesGridView->FileChosen = [wpView](std::shared_ptr<CShellFile>& spFile)->void {
			if (auto spView = wpView.lock()) {
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {//Open Filer
					auto& itemsSource = spView->GetItemsSource();
					itemsSource.replace(itemsSource.begin() + spView->GetSelectedIndex(), std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(spFile)));
				} else if (boost::iequals(spFile->GetPathExt(), L".txt")) {//Open Text
					auto& itemsSource = spView->GetItemsSource();
					itemsSource.replace(itemsSource.begin() + spView->GetSelectedIndex(), std::make_shared<TextTabData>(spFile->GetPath()));
				} else {//Open File
					spFile->Open();
				}
			}
		};
	};
	setUpFavoritesView(m_spLeftFavoritesView, std::weak_ptr<CFilerTabGridView>(m_spLeftView), CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"LeftFavoritesGridView"));
	setUpFavoritesView(m_spRightFavoritesView, std::weak_ptr<CFilerTabGridView>(m_spRightView), CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"RightFavoritesGridView"));

	//CFilerTabGridView
	auto setUpFilerTabGridView = [this](std::shared_ptr<CFilerTabGridView>& spView, unsigned short id)->void {
		spView->GetFilerGridViewPtr()->StatusLog = [this](const std::wstring& log) {
			m_spStatusBar->SetText(log);
			InvalidateRect(GetDirectPtr()->Dips2Pixels(m_spStatusBar->GetRectInWnd()), FALSE);
		};

	};

	setUpFilerTabGridView(m_spLeftView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"LeftFilerGridView"));
	setUpFilerTabGridView(m_spRightView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"RightFilerGridView"));

	//Context Menu
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

			for (auto iter = m_spExeExProp->ExeExtensions.cbegin(); iter != m_spExeExProp->ExeExtensions.cend(); ++iter) {

				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_TYPE | MIIM_ID;
				mii.fType = MFT_STRING;
				mii.fState = MFS_ENABLED;
				mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, std::get<ExeExtension>(*iter).Name);
				mii.dwTypeData = (LPWSTR)std::get<ExeExtension>(*iter).Name.c_str();
				menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
			}

		};
		spFilerView->ExecCustomContextMenu = [&](int idCmd, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl)->bool {
			if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToFavoritesFromItem")) {
				for (auto& pIdl : vpIdl) {
					STRRET strret;
					psf->GetDisplayNameOf(pIdl, SHGDN_FORPARSING, &strret);
					std::wstring path = shell::strret2wstring(strret, pIdl);
					GetFavoritesPropPtr()->GetFavorites().push_back(std::make_tuple(std::make_shared<CFavorite>(path, L"")));
					//m_spLeftFavoritesView->PushRow(std::make_shared<CBindRow<std::shared_ptr<CShellFile>>>(m_spLeftFavoritesView.get()));
					//m_spRightFavoritesView->PushRow(std::make_shared<CBindRow<std::shared_ptr<CShellFile>>>(m_spRightFavoritesView.get()));
				}
				m_spLeftFavoritesView->SubmitUpdate();
				m_spRightFavoritesView->SubmitUpdate();
				return true;
			} else {
				auto iter = std::find_if(m_spExeExProp->ExeExtensions.cbegin(), m_spExeExProp->ExeExtensions.cend(),
					[idCmd](const auto& exeex)->bool {
					return CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, std::get<ExeExtension>(exeex).Name) == idCmd;
				});
				if (iter != m_spExeExProp->ExeExtensions.cend()) {
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

	applyCustomContextMenu(m_spLeftView->GetFilerGridViewPtr());
	applyCustomContextMenu(m_spRightView->GetFilerGridViewPtr());
}

CFilerWnd::~CFilerWnd() = default;

HWND CFilerWnd::Create(HWND hWndParent)
{
	return CWnd::Create(hWndParent, m_rcWnd);
}

void CFilerWnd::OnPaint(const PaintEvent& e)
{
	GetDirectPtr()->FillSolidRectangle(CColorF(1.f, 1.f, 1.f), GetRectInWnd());
	CD2DWWindow::OnPaint(e);
	GetDirectPtr()->FillSolidRectangle(CColorF(1.f, 1.f, 1.f), m_splitterRect);
}

void CFilerWnd::OnCreate(const CreateEvt& e)
{
	//SetWindowPlacement make sure Window in Monitor
	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.rcNormalPosition = e.Rect;
	::SetWindowPlacement(m_hWnd, &wp);

	//Konami
	//m_konamiCommander.SetHwnd(m_hWnd);
	
	auto [rcLeftFav, rcLeftTab, rcSplitter, rcRightFav, rcRightTab, rcStatus] = GetRects();

	m_spLeftFavoritesView->OnCreate(CreateEvt(this, rcLeftFav));
	m_spLeftView->OnCreate(CreateEvt(this, rcLeftTab));
	m_splitterRect = rcSplitter;
	m_spRightFavoritesView->OnCreate(CreateEvt(this, rcRightFav));
	m_spRightView->OnCreate(CreateEvt(this, rcRightTab));
	m_spStatusBar->OnCreate(CreateEvt(this, rcStatus));

	SetFocusedControlPtr(m_spLeftView);
}

void CFilerWnd::OnKeyDown(const KeyDownEvent& e)
{
	*(e.HandledPtr) = FALSE;
	switch (e.Char)
	{
	case VK_F4:
		{
			if (::GetAsyncKeyState(VK_MENU)) {
				OnCommandExit(CommandEvent(this, 0, 0, e.HandledPtr));
			}
		}
		break;
	case VK_F5:
		{
			if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
				if (auto spCurFilerGrid = std::dynamic_pointer_cast<CFilerGridView>(spCurTab->GetCurrentControlPtr())) {
					std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
					if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerGridView>(spOtherTab->GetCurrentControlPtr())) {
						spCurFilerGrid->CopySelectedFilesTo(spOtherFilerGrid->GetFolder()->GetAbsoluteIdl());
						*(e.HandledPtr) = TRUE;
					}
				}
			}
		}
		break;
	case VK_F6:
		{
			if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
				if (auto spCurFilerGrid = std::dynamic_pointer_cast<CFilerGridView>(spCurTab->GetCurrentControlPtr())) {
					std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
					if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerGridView>(spOtherTab->GetCurrentControlPtr())) {
						spCurFilerGrid->MoveSelectedFilesTo(spOtherFilerGrid->GetFolder()->GetAbsoluteIdl());
						*(e.HandledPtr) = TRUE;
					}
				}
			}
		}
		break;
	case VK_F9:
		{
			if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
				if (auto spCurFilerGrid = std::dynamic_pointer_cast<CFilerGridView>(spCurTab->GetCurrentControlPtr())) {
					std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
					if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerGridView>(spOtherTab->GetCurrentControlPtr())) {
						spCurFilerGrid->CopyIncrementalSelectedFilesTo(spOtherFilerGrid->GetFolder()->GetAbsoluteIdl());
						*(e.HandledPtr) = TRUE;
					}
				}
			}
		}
		break;
	default:
		break;
	}
	if (!(*e.HandledPtr)) {
		CD2DWWindow::OnKeyDown(e);
	}
	//m_konamiCommander.OnKeyDown(uMsg, wParam, lParam, bHandled);
}

void CFilerWnd::OnClose(const CloseEvent& e)
{	
	WINDOWPLACEMENT wp={0};
	wp.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	m_rcWnd=CRect(wp.rcNormalPosition);
	
	CD2DWWindow::OnClose(e);
}

LRESULT CFilerWnd::OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	::PostQuitMessage(0);
	return 0;
}

void CFilerWnd::OnLButtonDown(const LButtonDownEvent& e)
{
	CRect leftRc(GetDirectPtr()->Dips2Pixels(m_spLeftView->GetRectInWnd()));

	m_isSizing = (e.PointInClient.x >= (leftRc.right)) &&
		(e.PointInClient.x <= (leftRc.right + kSplitterWidth));

	if (m_isSizing)
	{
		m_ptBeginClient = e.PointInClient;
		SetCapture();
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	} else {
		CD2DWWindow::OnLButtonDown(e);
	}
}

void CFilerWnd::OnLButtonUp(const LButtonUpEvent& e) 
{
	if (m_isSizing)
	{
		m_ptBeginClient.SetPoint(0, 0);
		m_isSizing = false;
		ReleaseCapture();
		OnRect(RectEvent(this, GetRectInWnd()));
	} else {
		CD2DWWindow::OnLButtonUp(e);
	}
}

void CFilerWnd::OnMouseMove(const MouseMoveEvent& e)
{
	CRect leftRc(GetDirectPtr()->Dips2Pixels(m_spLeftView->GetRectInWnd()));

	if ((e.PointInClient.x >= (leftRc.right)) &&
		(e.PointInClient.x <= (leftRc.right + kSplitterWidth)))
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
	}

	if (m_isSizing && e.Flags == MK_LBUTTON)
	{
		m_splitterLeft += e.PointInClient.x - m_ptBeginClient.x;
		m_ptBeginClient = e.PointInClient;
		OnRect(RectEvent(this, GetRectInWnd()));
	} else {
		CD2DWWindow::OnMouseMove(e);
	}
	//m_konamiCommander.OnMouseMove(uMsg, wParam, lParam, bHandled);
}

std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> CFilerWnd::GetRects()
{
	CRectF rcClient = GetRectInWnd();
	CRectF rcLeftFavorites, rcRightFavorites, rcLeftGrid, rcRightGrid, rcStatusBar;
	LONG favoriteWidth = GetDirectPtr()->Dips2PixelsX(
		m_spFilerGridViewProp->CellPropPtr->Line->Width * 2 + //def:GridLine=0.5*2, CellLine=0.5*2
		m_spFilerGridViewProp->CellPropPtr->Padding->left + //def:2
		m_spFilerGridViewProp->CellPropPtr->Padding->right + //def:2
		16.f);//icon
	LONG statusHeight = GetDirectPtr()->Dips2PixelsY(m_spStatusBar->MeasureSize(GetDirectPtr()).height);

	rcStatusBar.SetRect(
		rcClient.left, rcClient.bottom - statusHeight,
		rcClient.right, rcClient.bottom);

	rcLeftFavorites.SetRect(
		rcClient.left, rcClient.top,
		rcClient.left + favoriteWidth, rcClient.bottom - statusHeight);

	if (m_splitterLeft == 0) {//Initial = No serialize

		if (rcClient.Width() >= 800) {
			FLOAT viewWidth = (rcClient.Width() - 2 * favoriteWidth - kSplitterWidth) / 2;
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top,
				rcClient.left + favoriteWidth + viewWidth,
				rcClient.bottom - statusHeight);

			rcRightFavorites.SetRect(
				rcClient.left + favoriteWidth + viewWidth + kSplitterWidth,
				rcClient.top,
				rcClient.left + favoriteWidth + viewWidth + kSplitterWidth + favoriteWidth,
				rcClient.bottom - statusHeight);

			rcRightGrid.SetRect(
				rcClient.left + favoriteWidth + viewWidth + kSplitterWidth + favoriteWidth,
				rcClient.top,
				rcClient.right,
				rcClient.bottom - statusHeight);
			m_splitterLeft = favoriteWidth + viewWidth;
		} else {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top,
				rcClient.right,
				rcClient.bottom - statusHeight);
			m_splitterLeft = rcClient.right;
		}
	} else {
		if (rcClient.Width() >= m_splitterLeft + kSplitterWidth) {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top,
				m_splitterLeft,
				rcClient.bottom - statusHeight);

			rcRightFavorites.SetRect(
				m_splitterLeft + kSplitterWidth,
				rcClient.top,
				m_splitterLeft + kSplitterWidth + favoriteWidth,
				rcClient.bottom - statusHeight);

			rcRightGrid.SetRect(
				m_splitterLeft + kSplitterWidth + favoriteWidth,
				rcClient.top,
				rcClient.right,// - (m_splitterLeft + kSplitterWidth + favoriteWidth), 
				rcClient.bottom - statusHeight);
		} else {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top,
				rcClient.right,
				rcClient.bottom - statusHeight);
		}
	}

	rcLeftGrid.DeflateRect(2.f);
	rcRightGrid.DeflateRect(2.f);


	return {
		rcLeftFavorites, rcLeftGrid, CRectF(m_splitterLeft, rcClient.top, m_splitterLeft + kSplitterWidth, rcClient.bottom),
		rcRightFavorites, rcRightGrid, rcStatusBar
	};

}


//TODOTODO CWnd m_pdirect, create, erasebkgnd, onsize
void CFilerWnd::OnRect(const RectEvent& e)
{
	auto [rcLeftFav, rcLeftTab, rcSplitter, rcRightFav, rcRightTab, rcStatus] = GetRects();
	
	m_spLeftFavoritesView->OnRect(RectEvent(this, rcLeftFav));
	m_spRightFavoritesView->OnRect(RectEvent(this, rcRightFav));
	m_spStatusBar->OnRect(RectEvent(this, rcStatus));
	m_spLeftView->OnRect(RectEvent(this, rcLeftTab));
	m_spRightView->OnRect(RectEvent(this, rcRightTab));
	m_splitterRect = rcSplitter;
}

//void CFilerWnd::OnSetFocus(const SetFocusEvent& e)
//{
//	if (!m_spCurView) { m_spCurView = m_spLeftView; }
//	if(m_spCurView && m_spCurView->GetContentWnd()){
//		::SetFocus(m_spCurView->GetContentWnd()->m_hWnd);
//	}
//}

LRESULT CFilerWnd::OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_spLeftView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	//m_spRightView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	switch (wParam) {
		case DBT_DEVICEARRIVAL:
		case DBT_DEVICEREMOVECOMPLETE:
			CDriveFolderManager::GetInstance()->Update();
			m_spLeftFavoritesView->Reload();
			m_spRightFavoritesView->Reload();
			m_spLeftView->GetFilerGridViewPtr()->Reload();
			m_spRightView->GetFilerGridViewPtr()->Reload();
			InvalidateRect(NULL, FALSE);
		default:
			break;
	}
	return 0;
}

void CFilerWnd::OnCommandSave(const CommandEvent& e)
{
	SerializeProperty(this);
	*(e.HandledPtr) = TRUE;
}

void CFilerWnd::OnCommandExit(const CommandEvent& e)
{
	OnClose(CloseEvent(this, NULL, NULL, nullptr));
	*(e.HandledPtr) = TRUE;
}


void CFilerWnd::OnCommandApplicationOption(const CommandEvent& e)
{
	//TODOLOW
	//return OnCommandOption<CApplicationProperty>(L"Application Property", m_spApplicationProp,
	//	[this](const std::wstring& str)->void {
	//		SerializeProperty(this);
	//});
}

void CFilerWnd::OnCommandFilerGridViewOption(const CommandEvent& e)
{
	//TODOLOW
	//return OnCommandOption<FilerGridViewProperty>(L"FilerGridView Property", m_spFilerGridViewProp,
	//	[this](const std::wstring& str)->void {
	//	m_spLeftView->GetFilerGridViewPtr()->UpdateAll();
	//	m_spLeftView->GetToDoGridViewPtr()->UpdateAll();
	//	m_spRightView->GetFilerGridViewPtr()->UpdateAll();
	//	m_spRightView->GetToDoGridViewPtr()->UpdateAll();
	//	SerializeProperty(this);
	//});
}

void CFilerWnd::OnCommandTextOption(const CommandEvent& e)
{
	//TODOLOW
	//return OnCommandOption<TextEditorProperty>(L"Text Editor Property", m_spTextEditorProp,
	//	[this](const std::wstring& str)->void {
	//		m_spLeftView->GetTextViewPtr()->Update();
	//		m_spRightView->GetTextViewPtr()->Update();
	//		SerializeProperty(this);
	//	});
}


void CFilerWnd::OnCommandFavoritesOption(const CommandEvent& e)
{
	//TODOLOW
	//return OnCommandOption<CFavoritesProperty>(L"Favorites Property", m_spFavoritesProp,
	//	[this](const std::wstring& str)->void {
	//	m_spLeftFavoritesView->Reload();
	//	m_spRightFavoritesView->Reload();
	//	InvalidateRect(NULL, FALSE);
	//	SerializeProperty(this);
	//});
}

void CFilerWnd::OnCommandExeExtensionOption(const CommandEvent& e)
{
	//TODOLOW
	//return OnCommandOption<ExeExtensionProperty>(L"Exe extension Property", m_spExeExProp,
	//	[this](const std::wstring& str)->void {
	//	SerializeProperty(this);
	//});
}

#ifdef USE_PYTHON_EXTENSION
void CFilerWnd::OnCommandPythonExtensionOption(const CommandEvent& e)
{
	//TODOLOW
	return OnCommandOption<CPythonExtensionProperty>(L"Python extension Property", m_spPyExProp,
		[this](const std::wstring& str)->void {
		SerializeProperty(this);
	});
}
#endif


