#include "FilerApplication.h"
#include "FilerWnd.h"
#include "FilerGridView.h"
#include "Resource.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "GridViewProperty.h"
#include "FavoritesProperty.h"
#include "FavoritesGridView.h"
#include "LauncherGridView.h"
#include "FilerProperty.h"
#include "FilerTabGridView.h"
#include "MyFile.h"
#include "ViewProperty.h"
#include "FavoritesProperty.h"
#include "LauncherProperty.h"

#include "ResourceIDFactory.h"
#include "CellProperty.h"
#include "DriveFolder.h"
#include "ShellFunction.h"
#include "Direct2DWrite.h"
#include "ThreadSafeDriveFolderManager.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "BindCheckBoxColumn.h"
#include "BindCheckBoxCell.h"
#include "EditorProperty.h"

#include "ToDoGridView.h"
#include "D2DWWindow.h"
#include "MouseStateMachine.h"
#include <Dbt.h>
#include "PdfEditorProperty.h"
#include "ImageEditorProperty.h"

#include "FileOperationDlg.h"
#include "NetworkMessanger.h"

#ifdef USE_PYTHON_EXTENSION
#include "BoostPythonHelper.h"
#endif

std::vector<std::wstring> CFilerWnd::imageExts = { L".bmp", L".gif", L".ico", L".jpg", L".jpeg", L".png",L".tiff" };
std::vector<std::wstring> CFilerWnd::previewExts = {L".docx", L".doc", L".xlsx", L".xls", L".ppt", L".pptx"};


CFilerWnd::CFilerWnd()
	:CD2DWWindow(),
	m_rcWnd(0, 0, 1000, 600), 
	m_rcPropWnd(0, 0, 300, 400),
	SplitterLeft(0.f),
	m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	m_spDialogProp(std::make_shared<DialogProperty>()),
	m_spFilerGridViewProp(std::make_shared<FilerGridViewProperty>()),
	m_spEditorProp(std::make_shared<EditorProperty>()),
	m_spPdfEditorProp(std::make_shared<PDFEditorProperty>()),
	m_spImageEditorProp(std::make_shared<ImageEditorProperty>()),
	m_spPreviewControlProp(std::make_shared<PreviewControlProperty>()),
	
	m_spStatusBarProp(std::make_shared<StatusBarProperty>()),
	m_spTabControlProp(std::make_shared<TabControlProperty>()),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	m_spLauncherProp(std::make_shared<CLauncherProperty>()),
	m_spExeExProp(std::make_shared<ExeExtensionProperty>()),
	m_spSplitterProp(std::make_shared<SplitterProperty>()),
	m_spLauncher(std::make_shared<CLauncherGridView>(this, m_spFilerGridViewProp, m_spLauncherProp)),
	m_spToolBar(std::make_shared<CToolBar>(this)),
	m_spLeftView(std::make_shared<CFilerTabGridView>(this, m_spTabControlProp, m_spFilerGridViewProp, m_spEditorProp, m_spPdfEditorProp, m_spImageEditorProp, m_spPreviewControlProp)),
	m_spRightView(std::make_shared<CFilerTabGridView>(this, m_spTabControlProp, m_spFilerGridViewProp, m_spEditorProp, m_spPdfEditorProp, m_spImageEditorProp, m_spPreviewControlProp)),
	m_spSplitter(std::make_shared<CHorizontalSplitter>(this, m_spLeftView.get(), m_spRightView.get(), m_spSplitterProp)),
	m_spLeftFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFilerGridViewProp, m_spFavoritesProp)),
	m_spRightFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFilerGridViewProp, m_spFavoritesProp)),
	m_spStatusBar(std::make_shared<CFilerWndStatusBar>(this, m_spStatusBarProp)),
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
	AddMsgHandler(CNetworkMessanger::WM_CONNECTIVITYCHANGED, &CFilerWnd::OnConnectivityChanged, this);
	AddMsgHandler(WM_DEVICECHANGE, &CFilerWnd::OnDeviceChange, this);

	m_commandMap.emplace(IDM_SAVE, std::bind(&CFilerWnd::OnCommandSave, this, phs::_1));
	m_commandMap.emplace(IDM_EXIT, std::bind(&CFilerWnd::OnCommandExit, this, phs::_1));
	m_commandMap.emplace(IDM_APPLICATIONOPTION, std::bind(&CFilerWnd::OnCommandApplicationOption, this, phs::_1));
	m_commandMap.emplace(IDM_FILERGRIDVIEWOPTION, std::bind(&CFilerWnd::OnCommandFilerGridViewOption, this, phs::_1));
	m_commandMap.emplace(IDM_TEXTOPTION, std::bind(&CFilerWnd::OnCommandTextOption, this, phs::_1));
	m_commandMap.emplace(IDM_LAUNCHEROPTION, std::bind(&CFilerWnd::OnCommandLauncherOption,this, phs::_1));
	m_commandMap.emplace(IDM_FAVORITESOPTION, std::bind(&CFilerWnd::OnCommandFavoritesOption,this, phs::_1));
	m_commandMap.emplace(IDM_EXEEXTENSIONOPTION, std::bind(&CFilerWnd::OnCommandExeExtensionOption, this, phs::_1));
}

CFilerWnd::~CFilerWnd() = default;

HWND CFilerWnd::Create(HWND hWndParent)
{
	SplitterLeft.binding(m_spSplitter->SplitterLeft);
	HWND hWnd = CWnd::Create(hWndParent, m_rcWnd);
	return hWnd;
}

void CFilerWnd::OnPaint(const PaintEvent& e)
{
	GetDirectPtr()->FillSolidRectangle(CColorF(1.f, 1.f, 1.f), GetRectInWnd());
	CD2DWWindow::OnPaint(e);
}

void CFilerWnd::SetUpPreview(const std::shared_ptr<CFilerTabGridView>& subject, const std::shared_ptr<CFilerTabGridView>& observer)
{
	subject->GetFilerGridViewPtr()->SelectedItem.subscribe([this, observer](const std::shared_ptr<CShellFile>& spFile) {

		if (!m_isPreview) return;

		std::shared_ptr<TabData> spObsData = observer->ItemsSource.get_unconst()->at(*observer->SelectedIndex);

		//Text
		if (auto spTxtData = std::dynamic_pointer_cast<TextTabData>(spObsData);
			spTxtData && boost::iequals(spFile->GetPathExt(), L".txt")) {
			spTxtData->Doc.get_unconst()->Open(spFile->GetPath());
		//PDF
		} else if (auto spPdfData = std::dynamic_pointer_cast<PdfTabData>(spObsData);
			spPdfData && boost::iequals(spFile->GetPathExt(), L".pdf")) {
			CPDFDoc newDoc;
			newDoc.Open(spFile->GetPath());
			spPdfData->Scale.set(-1);
			spPdfData->VScroll.set(0.f);
			spPdfData->HScroll.set(0.f);
			spPdfData->Doc.set(newDoc);
		//Image
		} else if (auto spImgData = std::dynamic_pointer_cast<ImageTabData>(spObsData);
			spImgData && std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
			spImgData->Image.get_unconst()->Open(spFile->GetPath());
		//Preview
		} else if (auto spPrvData = std::dynamic_pointer_cast<PreviewTabData>(spObsData);
			spPrvData && std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
			// TODO PreviewControl to Open
		}

		//std::shared_ptr<TabData> spNewData;
		////Text
		//if (auto spTxtData = std::dynamic_pointer_cast<TextTabData>(spObsData);
		//	spTxtData && boost::iequals(spFile->GetPathExt(), L".txt")) {
		//	spNewData = std::make_shared<TextTabData>(spFile->GetPath());
		////PDF
		//} else if (auto spPdfData = std::dynamic_pointer_cast<PdfTabData>(spObsData);
		//	spPdfData && boost::iequals(spFile->GetPathExt(), L".pdf")) {
		//	spNewData = std::make_shared<PdfTabData>(spFile->GetPath());
		////Image
		//} else if (auto spImgData = std::dynamic_pointer_cast<ImageTabData>(spObsData);
		//	spImgData && std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
		//	spNewData = std::make_shared<ImageTabData>(GetWndPtr()->GetDirectPtr(), spFile->GetPath());
		////Preview
		//} else if (auto spPrvData = std::dynamic_pointer_cast<PreviewTabData>(spObsData);
		//	spPrvData && std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
		//	spNewData = std::make_shared<PreviewTabData>(spFile->GetPath());
		//}

		//if (spNewData) {
		//	//Replace
		//	if (spObsData->AcceptClosing(this, false)) {
		//		observer->ItemsSource.replace(observer->ItemsSource.get_unconst()->begin() + *observer->SelectedIndex, spNewData);
		//	}
		//}


	}, shared_from_this());
}


void CFilerWnd::OnCreate(const CreateEvt& e)
{
	m_pNetworkMessanger = std::make_unique<CNetworkMessanger>(m_hWnd);

#ifdef USE_PYTHON_EXTENSION
	m_commandMap.emplace(IDM_PYTHONEXTENSIONOPTION, std::bind(&CFilerWnd::OnCommandPythonExtensionOption, this, phs::_1));
#endif
	//CLauncherGridView
	//Do nothing

	//CFavoritesGridView
	auto setUpFavoritesView = [this](
		const std::shared_ptr<CFavoritesGridView>& spFavoritesGridView,
		const std::weak_ptr<CFilerTabGridView>& wpView)->void {
		spFavoritesGridView->FileChosen = [wpView, this](const std::shared_ptr<CShellFile>& spFile)->void {
			if (auto spView = wpView.lock()) {
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {//Open Filer
					auto& itemsSource = spView->ItemsSource;
					if (itemsSource->at(*spView->SelectedIndex)->AcceptClosing(GetWndPtr(), false)) {
						itemsSource.replace(itemsSource.get_unconst()->begin() + *spView->SelectedIndex, std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(spFile)));
					} else {
						itemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(spFile)));
					}
				} else if (boost::iequals(spFile->GetPathExt(), L".txt")) {//Open Text
					auto& itemsSource = spView->ItemsSource;
					if (itemsSource->at(*spView->SelectedIndex)->AcceptClosing(GetWndPtr(), false)) {
						itemsSource.replace(itemsSource.get_unconst()->begin() + *spView->SelectedIndex, std::make_shared<TextTabData>(spFile->GetPath()));
					} else {
						itemsSource.push_back(std::make_shared<TextTabData>(spFile->GetPath()));
					}
				} else {//Open File
					spFile->Open();
				}
			}
		};
	};
	setUpFavoritesView(m_spLeftFavoritesView, std::weak_ptr<CFilerTabGridView>(m_spLeftView));
	setUpFavoritesView(m_spRightFavoritesView, std::weak_ptr<CFilerTabGridView>(m_spRightView));

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
			mii.dwTypeData = const_cast<LPWSTR>(L"Add to favorite");
			menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

			menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);
			mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToLauncherFromItem");
			mii.dwTypeData = const_cast<LPWSTR>(L"Add to launcher");
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
			//PDF
			{
				menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);
				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_TYPE | MIIM_ID;
				mii.fType = MFT_STRING;
				mii.fState = MFS_ENABLED;
				mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFSplit");
				mii.dwTypeData = (LPWSTR)L"PDF Split";
				menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

				mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFMerge");
				mii.dwTypeData = (LPWSTR)L"PDF Merge";
				menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

				mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFExtract");
				mii.dwTypeData = (LPWSTR)L"PDF Extract";
				menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

				mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFUnlock");
				mii.dwTypeData = (LPWSTR)L"PDF Unlock";
				menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
			}


			menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);

			for (auto iter = m_spExeExProp->ExeExtensions->cbegin(); iter != m_spExeExProp->ExeExtensions->cend(); ++iter) {

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
		spFilerView->ExecCustomContextMenu = [&](int idCmd, const std::shared_ptr<CShellFolder>& folder, const std::vector<std::shared_ptr<CShellFile>>& files)->bool {
			if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToFavoritesFromItem")) {
				for (auto& file : files) {
					GetFavoritesPropPtr()->Favorites.push_back(std::make_shared<CFavorite>(file->GetPath(), L""));
				}
				m_spLeftFavoritesView->SubmitUpdate();
				m_spRightFavoritesView->SubmitUpdate();
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToLauncherFromItem")) {
				for (auto& file : files) {
					GetLauncherPropPtr()->Launchers.push_back(std::make_shared<CLauncher>(file->GetPath(), L""));
				}
				m_spLauncher->SubmitUpdate();
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFSplit")) {
				auto spDlg = std::make_shared<CPDFSplitDlg>(
					this,
					GetDialogPropPtr(),
					m_spFilerGridViewProp, m_spEditorProp->EditorTextBoxPropPtr, folder, files);

				spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 200))));
				GetWndPtr()->SetFocusToControl(spDlg);
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFMerge")) {
				auto spDlg = std::make_shared<CPDFMergeDlg>(
					this,
					GetDialogPropPtr(),
					m_spFilerGridViewProp, m_spEditorProp->EditorTextBoxPropPtr, folder, files);

				spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg);
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFExtract")) {
				auto spDlg = std::make_shared<CPDFExtractDlg>(
					this,
					GetDialogPropPtr(),
					m_spFilerGridViewProp, m_spEditorProp->EditorTextBoxPropPtr, folder, files);

				spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg);
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFUnlock")) {
				auto spDlg = std::make_shared<CPDFUnlockDlg>(
					this,
					GetDialogPropPtr(),
					m_spFilerGridViewProp, m_spEditorProp->EditorTextBoxPropPtr, folder, files);

				spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg);
				return true;
			} else {
				auto iter = std::find_if(m_spExeExProp->ExeExtensions->cbegin(), m_spExeExProp->ExeExtensions->cend(),
					[idCmd](const auto& exeex)->bool {
					return CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, std::get<ExeExtension>(exeex).Name) == idCmd;
				});
				if (iter != m_spExeExProp->ExeExtensions->cend()) {
					auto spDlg = std::make_shared<CExeExtensionDlg>(
						this,
						GetDialogPropPtr(),
						m_spFilerGridViewProp, m_spEditorProp->EditorTextBoxPropPtr, folder, files, std::get<ExeExtension>(*iter));

					spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 400))));
					GetWndPtr()->SetFocusToControl(spDlg);
					return true;
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

	//SetWindowPlacement make sure Window in Monitor
	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.rcNormalPosition = e.Rect;
	::SetWindowPlacement(m_hWnd, &wp);

	//Konami
	//m_konamiCommander.SetHwnd(m_hWnd);
	


	auto [rcLauncher, rcToolBar, rcLeftFav, rcLeftTab, rcSplitter, rcRightFav, rcRightTab, rcStatus] = GetRects();

	m_spLauncher->OnCreate(CreateEvt(this, this, rcLauncher));
	m_spToolBar->OnCreate(CreateEvt(this, this, rcToolBar));
	auto spBtn = std::make_shared<CButton>(m_spToolBar.get(), std::make_shared<ButtonProperty>());
	spBtn->Content.set(m_isPreview?L"Prv":L"Nrm");
	spBtn->Command.subscribe([this, spBtn]() {
		m_isPreview = !m_isPreview;
		spBtn->Content.set(m_isPreview ? L"Prv" : L"Nrm");
	}, shared_from_this());
	spBtn->OnCreate(CreateEvt(this, m_spToolBar.get(), CRectF()));

	m_spLeftFavoritesView->OnCreate(CreateEvt(this, this, rcLeftFav));
	m_spLeftView->OnCreate(CreateEvt(this, this, rcLeftTab));
	m_spSplitter->OnCreate(CreateEvt(this, this, rcSplitter));
	m_spRightFavoritesView->OnCreate(CreateEvt(this, this, rcRightFav));
	m_spRightView->OnCreate(CreateEvt(this, this, rcRightTab));
	m_spStatusBar->OnCreate(CreateEvt(this, this, rcStatus));
	m_spLeftView->SetIsTabStop(true);
	m_spRightView->SetIsTabStop(true);

	SetUpPreview(m_spLeftView, m_spRightView);
	SetUpPreview(m_spRightView, m_spLeftView);

	GetWndPtr()->SetFocusToControl(m_spLeftView);
}

void CFilerWnd::OnKeyDown(const KeyDownEvent& e)
{
	*(e.HandledPtr) = FALSE;
	switch (e.Char)
	{
	case 'Q':
		//Replace or PushBack
		if (::GetAsyncKeyState(VK_CONTROL)) {
			if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
				if (auto spCurFilerGrid = std::dynamic_pointer_cast<CFilerGridView>(spCurTab->GetCurrentControlPtr())) {
					std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
					std::shared_ptr<TabData> spNewData;
					if (boost::iequals(spCurFilerGrid->GetFocusedFile()->GetPathExt(), L".txt")) {
						spNewData = std::make_shared<TextTabData>(spCurFilerGrid->GetFocusedFile()->GetPath());
					} else if (boost::iequals(spCurFilerGrid->GetFocusedFile()->GetPathExt(), L".pdf")) {
						spNewData = std::make_shared<PdfTabData>(spCurFilerGrid->GetFocusedFile()->GetPath());
					} else if (std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spCurFilerGrid->GetFocusedFile()->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
						spNewData = std::make_shared<ImageTabData>(GetWndPtr()->GetDirectPtr(), spCurFilerGrid->GetFocusedFile()->GetPath());
					} else if (std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spCurFilerGrid->GetFocusedFile()->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
						spNewData = std::make_shared<PreviewTabData>(spCurFilerGrid->GetFocusedFile()->GetPath());
					}

					if (spNewData) {
						//Replace
						if (::GetAsyncKeyState(VK_SHIFT) && spOtherTab->ItemsSource.get_unconst()->at(*spOtherTab->SelectedIndex)->AcceptClosing(this, false)) {
							spOtherTab->ItemsSource.replace(spOtherTab->ItemsSource.get_unconst()->begin() + *spOtherTab->SelectedIndex, spNewData);
						//Push back	
						} else {
							spOtherTab->ItemsSource.push_back(spNewData);
						}
						*(e.HandledPtr) = TRUE;
					}
				}
			}
		}
		break;

	case VK_F4:
		{
			if (::GetAsyncKeyState(VK_MENU)) {
				OnCommandExit(CommandEvent(this, 0, 0, e.HandledPtr));
				*(e.HandledPtr) = TRUE;
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
	CD2DWWindow::OnLButtonDown(e);
}

void CFilerWnd::OnLButtonUp(const LButtonUpEvent& e) 
{
	CD2DWWindow::OnLButtonUp(e);
}

void CFilerWnd::OnMouseMove(const MouseMoveEvent& e)
{
	CD2DWWindow::OnMouseMove(e);
	//m_konamiCommander.OnMouseMove(uMsg, wParam, lParam, bHandled);
}

std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> CFilerWnd::GetRects()
{
	CRectF rcClient = GetRectInWnd();

	LONG launcherHeight = GetDirectPtr()->Dips2PixelsX(
		m_spFilerGridViewProp->CellPropPtr->Line->Width * 2 + //def:GridLine=0.5*2, CellLine=0.5*2
		m_spFilerGridViewProp->CellPropPtr->Padding->left + //default:2
		m_spFilerGridViewProp->CellPropPtr->Padding->right + //default:2
		16.f);//icon

	LONG favoriteWidth = GetDirectPtr()->Dips2PixelsX(
		m_spFilerGridViewProp->CellPropPtr->Line->Width * 2 + //def:GridLine=0.5*2, CellLine=0.5*2
		m_spFilerGridViewProp->CellPropPtr->Padding->left + //default:2
		m_spFilerGridViewProp->CellPropPtr->Padding->right + //default:2
		16.f);//icon
	LONG statusHeight = GetDirectPtr()->Dips2PixelsY(m_spStatusBar->MeasureSize(L"").height);

	CRectF rcLauncher, rcToolBar, rcLeftFavorites, rcRightFavorites, rcSplitter, rcLeftGrid, rcRightGrid, rcStatusBar;

	rcLauncher.SetRect(rcClient.left, rcClient.top, (rcClient.left + rcClient.right)*0.5f, rcClient.top + launcherHeight);

	m_spToolBar->Measure(CSizeF(FLT_MAX, FLT_MAX));
	rcToolBar.SetRect((rcClient.left + rcClient.right)*0.5f, rcClient.top, rcClient.right , rcClient.top + launcherHeight);

	rcStatusBar.SetRect(rcClient.left, rcClient.bottom - statusHeight, rcClient.right, rcClient.bottom);

	rcLeftFavorites.SetRect(
		rcClient.left, rcClient.top + launcherHeight,
		rcClient.left + favoriteWidth, rcClient.bottom - statusHeight);

	if (*SplitterLeft == 0) {//Initial = No serialize

		if (rcClient.Width() >= 800) {
			FLOAT viewWidth = (rcClient.Width() - 2 * favoriteWidth - m_spSplitterProp->Width) / 2;
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top + launcherHeight,
				rcClient.left + favoriteWidth + viewWidth,
				rcClient.bottom - statusHeight);

			rcRightFavorites.SetRect(
				rcClient.left + favoriteWidth + viewWidth + m_spSplitterProp->Width,
				rcClient.top  + launcherHeight,
				rcClient.left + favoriteWidth + viewWidth + m_spSplitterProp->Width + favoriteWidth,
				rcClient.bottom - statusHeight);

			rcRightGrid.SetRect(
				rcClient.left + favoriteWidth + viewWidth + m_spSplitterProp->Width + favoriteWidth,
				rcClient.top  + launcherHeight,
				rcClient.right,
				rcClient.bottom - statusHeight);
			SplitterLeft.set(favoriteWidth + viewWidth);
		} else {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top  + launcherHeight,
				rcClient.right,
				rcClient.bottom - statusHeight);
			SplitterLeft.set(rcClient.right);
		}
	} else {
		if (rcClient.Width() >= *SplitterLeft + m_spSplitterProp->Width) {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top  + launcherHeight,
				*SplitterLeft,
				rcClient.bottom - statusHeight);

			rcRightFavorites.SetRect(
				*SplitterLeft + m_spSplitterProp->Width,
				rcClient.top + launcherHeight,
				*SplitterLeft + m_spSplitterProp->Width + favoriteWidth,
				rcClient.bottom - statusHeight);

			rcRightGrid.SetRect(
				*SplitterLeft + m_spSplitterProp->Width + favoriteWidth,
				rcClient.top + launcherHeight,
				rcClient.right,// - (m_splitterLeft + kSplitterWidth + favoriteWidth), 
				rcClient.bottom - statusHeight);
		} else {
			rcLeftGrid.SetRect(
				rcClient.left + favoriteWidth,
				rcClient.top + launcherHeight,
				rcClient.right,
				rcClient.bottom - statusHeight);
		}
	}

	rcLeftGrid.DeflateRect(2.f);
	rcRightGrid.DeflateRect(2.f);
	rcSplitter.SetRect(*SplitterLeft, rcClient.top + launcherHeight, *SplitterLeft + m_spSplitterProp->Width, rcClient.bottom);

	return {
		rcLauncher,
		rcToolBar,
		rcLeftFavorites,
		rcLeftGrid,
		rcSplitter,
		rcRightFavorites, 
		rcRightGrid,
		rcStatusBar
	};

}


//TODOTODO CWnd m_pdirect, create, erasebkgnd, onsize
void CFilerWnd::OnRect(const RectEvent& e)
{
	auto [rcLauncher, rcToolBar, rcLeftFav, rcLeftTab, rcSplitter, rcRightFav, rcRightTab, rcStatus] = GetRects();
	
	m_spLauncher->OnRect(RectEvent(this, rcLauncher));
	m_spToolBar->Arrange(rcToolBar);
	m_spLeftFavoritesView->OnRect(RectEvent(this, rcLeftFav));
	m_spRightFavoritesView->OnRect(RectEvent(this, rcRightFav));
	m_spStatusBar->OnRect(RectEvent(this, rcStatus));
	m_spSplitter->OnRect(RectEvent(this, rcSplitter));
	m_spLeftView->OnRect(RectEvent(this, rcLeftTab));
	m_spRightView->OnRect(RectEvent(this, rcRightTab));
}

//void CFilerWnd::OnSetFocus(const SetFocusEvent& e)
//{
//	if (!m_spCurView) { m_spCurView = m_spLeftView; }
//	if(m_spCurView && m_spCurView->GetContentWnd()){
//		::SetFocus(m_spCurView->GetContentWnd()->m_hWnd);
//	}
//}
LRESULT CFilerWnd::OnConnectivityChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CDriveFolderManager::GetInstance()->Update();
	m_spLauncher->Reload();
	m_spLeftFavoritesView->Reload();
	m_spRightFavoritesView->Reload();
	m_spLeftView->GetFilerGridViewPtr()->Reload();
	m_spRightView->GetFilerGridViewPtr()->Reload();
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CFilerWnd::OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_spLeftView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	//m_spRightView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	switch (wParam) {
		case DBT_DEVICEARRIVAL:
		case DBT_DEVICEREMOVECOMPLETE:
			CDriveFolderManager::GetInstance()->Update();
			m_spLauncher->Reload();
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
	CFilerApplication::GetInstance()->Serialize();
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

void CFilerWnd::OnCommandLauncherOption(const CommandEvent& e)
{
	m_spLauncherProp = CFilerApplication::GetInstance()->DeserializeLauncher();
	m_spLauncher->ItemsSource = m_spLauncherProp->Launchers;

	m_spLauncher->Reload();
	InvalidateRect(NULL, FALSE);
}


void CFilerWnd::OnCommandFavoritesOption(const CommandEvent& e)
{
	m_spFavoritesProp = CFilerApplication::GetInstance()->DeserializeFavoirtes();
	m_spLeftFavoritesView->ItemsSource = m_spFavoritesProp->Favorites;
	m_spRightFavoritesView->ItemsSource = m_spFavoritesProp->Favorites;

	m_spLeftFavoritesView->Reload();
	m_spRightFavoritesView->Reload();
	InvalidateRect(NULL, FALSE);
}

void CFilerWnd::OnCommandExeExtensionOption(const CommandEvent& e)
{
	m_spExeExProp = CFilerApplication::GetInstance()->DeserializeExeExtension();
	InvalidateRect(NULL, FALSE);
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


