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

#include "ThreadPool.h"

#include "FileOperationDlg.h"
#include "NetworkMessanger.h"
#include "Dispatcher.h"
#include "Performance.h"

#include "SplitContainer.h"

#ifdef USE_PYTHON_EXTENSION
#include "BoostPythonHelper.h"
#endif

std::vector<std::wstring> CFilerWnd::imageExts = { L".bmp", L".gif", L".ico", L".jpg", L".jpeg", L".png",L".tiff" };
std::vector<std::wstring> CFilerWnd::previewExts = {L".docx", L".doc", L".xlsx", L".xls", L".ppt", L".pptx"};


CFilerWnd::CFilerWnd()
	:CD2DWWindow(),
	//m_reloadIosv(), m_reloadWork(m_reloadIosv), m_reloadTimer(m_reloadIosv),
	Rectangle(0, 0, 1000, 600), 
	m_pPerformance(std::make_unique<CPerformance>()),
	m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	m_spLauncherProp(std::make_shared<CLauncherProperty>()),
	m_spExeExProp(std::make_shared<ExeExtensionProperty>()),
	m_spLauncher(std::make_shared<CLauncherGridView>(this, m_spLauncherProp)),
	m_spToolBar(std::make_shared<CToolBar>(this)),
	m_spHorizontalSplit(std::make_shared<CHorizontalSplitContainer>(this)),
	//m_spTopVerticalSplit(std::make_shared<CVerticalSplitContainer>(this)),
	//m_spBottomVerticalSplit(std::make_shared<CVerticalSplitContainer>(this)),
	//m_spLeftView(std::make_shared<CFilerTabGridView>(this)),
	//m_spRightView(std::make_shared<CFilerTabGridView>(this)),
	//m_spLeftFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFavoritesProp)),
	//m_spRightFavoritesView(std::make_shared<CFavoritesGridView>(this, m_spFavoritesProp)),
	//m_spLogText(std::make_shared<CColoredTextBox>(this, L"")),
	m_spStatusBar(std::make_shared<CStatusBar>(this))
	//m_spHSplitter(std::make_shared<CHorizontalSplitter>(this)),
	//m_spVSplitter(std::make_shared<CVerticalSplitter>(this)),
	//m_spCurView(m_spLeftView)
#ifdef USE_PYTHON_EXTENSION
	,m_spPyExProp(std::make_shared<CPythonExtensionProperty>())
#endif
{
	//Top Vertical
	auto spTopVerticalSplit = std::make_shared<CVerticalSplitContainer>(m_spHorizontalSplit.get());
	auto spTabLeft = std::make_shared<CFilerTabGridView>(spTopVerticalSplit.get());
	auto spTabRight = std::make_shared<CFilerTabGridView>(spTopVerticalSplit.get());
	spTabLeft->SetIsTabStop(true);
	spTabRight->SetIsTabStop(true);
	SetUpPreview(spTabLeft, spTabRight);
	SetUpPreview(spTabRight, spTabLeft);
	spTopVerticalSplit->SetLeft(spTabLeft);
	spTopVerticalSplit->SetRight(spTabRight);

	//Bottom Vertical
	auto spBottomVerticalSplit = std::make_shared<CVerticalSplitContainer>(m_spHorizontalSplit.get());
	auto spTextBoxLeft = std::make_shared<CColoredTextBox>(spBottomVerticalSplit.get(), L"");
	auto spTextBoxRight = std::make_shared<CColoredTextBox>(spBottomVerticalSplit.get(), L"");
	PerformanceLog.binding(spTextBoxLeft->Text);
	ThreadPoolLog.binding(spTextBoxRight->Text);
	spBottomVerticalSplit->SetLeft(spTextBoxLeft);
	spBottomVerticalSplit->SetRight(spTextBoxRight);

	//Horizontal
	m_spHorizontalSplit->SetTop(spTopVerticalSplit);
	m_spHorizontalSplit->SetTop(spBottomVerticalSplit);


	//std::thread th(boost::bind(&boost::asio::io_service::run, &m_reloadIosv));
	//th.detach();


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
		.x(Rectangle->left)
		.y(Rectangle->top)
		.nWidth(Rectangle->Width())
		.nHeight(Rectangle->Height())
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
	HWND hWnd = CWnd::Create(hWndParent, *Rectangle);
	return hWnd;
}

void CFilerWnd::OnPaint(const PaintEvent& e)
{
	GetDirectPtr()->FillSolidRectangle(CColorF(1.f, 1.f, 1.f), GetRectInWnd());
	CD2DWWindow::OnPaint(e);
}

void CFilerWnd::SetUpPreview(const std::shared_ptr<CFilerTabGridView>& subject, const std::shared_ptr<CFilerTabGridView>& observer)
{
	subject->GetFilerViewPtr()->GetFileGridPtr()->SelectedItem.subscribe([this, wp = std::weak_ptr(observer)](const std::shared_ptr<CShellFile>& spFile) {

		if (auto observer = wp.lock()) {
			if (!m_isPreview) return;

			std::shared_ptr<TabData> spObsData = observer->ItemsSource.get_unconst()->at(*observer->SelectedIndex);

			//Text
			if (auto spTxtData = std::dynamic_pointer_cast<TextTabData>(spObsData);
				spTxtData && boost::iequals(spFile->GetPathExt(), L".txt")) {
				//TODO
			//PDF
			} else if (auto spPdfData = std::dynamic_pointer_cast<PdfTabData>(spObsData);
				spPdfData && boost::iequals(spFile->GetPathExt(), L".pdf")) {
				std::shared_ptr<CPDFDoc> newDoc;
				newDoc->Open(spFile->GetPath());
				spPdfData->Scale.set(-1);
				spPdfData->VScroll.set(0.f);
				spPdfData->HScroll.set(0.f);
				spPdfData->Doc.set(newDoc);
				//Image
			} else if (auto spImgData = std::dynamic_pointer_cast<ImageTabData>(spObsData);
				spImgData && std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
				CD2DImage newDoc(spFile->GetPath());
				spImgData->Scale.set(-1);
				spImgData->VScroll.set(0.f);
				spImgData->HScroll.set(0.f);
				spImgData->Image.set(newDoc);
				//Preview
			} else if (auto spPrvData = std::dynamic_pointer_cast<PreviewTabData>(spObsData);
				spPrvData && std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
				CShellFile newDoc(spFile->GetPath());
				//spPrvData->Scale.set(-1);
				//spPrvData->VScroll.set(0.f);
				//spPrvData->HScroll.set(0.f);
				spPrvData->Doc.set(newDoc);
			}
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

	////CFavoritesGridView
	//auto setUpFavoritesView = [this](
	//	const std::shared_ptr<CFavoritesGridView>& spFavoritesGridView,
	//	const std::weak_ptr<CFilerTabGridView>& wpView)->void {
	//	spFavoritesGridView->FileChosen = [wpView, this](const std::shared_ptr<CShellFile>& spFile)->void {
	//		if (auto spView = wpView.lock()) {
	//			if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {//Open Filer
	//				auto& itemsSource = spView->ItemsSource;
	//				if (itemsSource->at(*spView->SelectedIndex)->AcceptClosing(GetWndPtr(), false)) {
	//					itemsSource.replace(itemsSource.get_unconst()->begin() + *spView->SelectedIndex, std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(spFile)));
	//				} else {
	//					itemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(spFile)));
	//				}
	//			} else if (boost::iequals(spFile->GetPathExt(), L".txt")) {//Open Text
	//				auto& itemsSource = spView->ItemsSource;
	//				if (itemsSource->at(*spView->SelectedIndex)->AcceptClosing(GetWndPtr(), false)) {
	//					itemsSource.replace(itemsSource.get_unconst()->begin() + *spView->SelectedIndex, std::make_shared<TextTabData>(spFile->GetPath()));
	//				} else {
	//					itemsSource.push_back(std::make_shared<TextTabData>(spFile->GetPath()));
	//				}
	//			} else {//Open File
	//				spFile->Open();
	//			}
	//		}
	//	};
	//};
	//setUpFavoritesView(m_spLeftFavoritesView, std::weak_ptr<CFilerTabGridView>(m_spLeftView));
	//setUpFavoritesView(m_spRightFavoritesView, std::weak_ptr<CFilerTabGridView>(m_spRightView));

	//CFilerTabGridView
	// TODOTODO
	//auto setUpFilerTabGridView = [this](std::shared_ptr<CFilerTabGridView>& spView, unsigned short id)->void {
	//	spView->GetFilerViewPtr()->GetFileGridPtr()->StatusLog = [this](const std::wstring& log) {
	//		m_spStatusBar->Text.set(log);
	//		InvalidateRect(NULL, FALSE);
	//	};

	//};

	//setUpFilerTabGridView(m_spLeftView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"LeftFilerGridView"));
	//setUpFilerTabGridView(m_spRightView, CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"RightFilerGridView"));

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
				// TODOTODO
				//m_spLeftView->GetFilerViewPtr()->GetFavoriteGridPtr()->SubmitUpdate();
				//m_spLeftView->GetFilerViewPtr()->GetFavoriteGridPtr()->SubmitUpdate();
				//m_spLeftFavoritesView->SubmitUpdate();
				//m_spRightFavoritesView->SubmitUpdate();
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"AddToLauncherFromItem")) {
				for (auto& file : files) {
					GetLauncherPropPtr()->Launchers.push_back(std::make_shared<CLauncher>(file->GetPath(), L""));
				}
				m_spLauncher->SubmitUpdate();
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFSplit")) {
				auto spDlg = std::make_shared<CPDFSplitDlg>(
					this, folder, files);

				spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 200))));
				GetWndPtr()->SetFocusToControl(spDlg);
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFMerge")) {
				auto spDlg = std::make_shared<CPDFMergeDlg>(
					this, folder, files);

				spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg);
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFExtract")) {
				auto spDlg = std::make_shared<CPDFExtractDlg>(
					this, folder, files);

				spDlg->OnCreate(CreateEvt(this, this, CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg);
				return true;
			} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"PDFUnlock")) {
				auto spDlg = std::make_shared<CPDFUnlockDlg>(
					this, folder, files);

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
						this, folder, files, std::get<ExeExtension>(*iter));

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

	// TODOTODO
	//applyCustomContextMenu(m_spLeftView->GetFilerViewPtr()->GetFileGridPtr());
	//applyCustomContextMenu(m_spRightView->GetFilerViewPtr()->GetFileGridPtr());

	//SetWindowPlacement make sure Window in Monitor
	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.rcNormalPosition = e.Rect;
	::SetWindowPlacement(m_hWnd, &wp);

	//Konami
	//m_konamiCommander.SetHwnd(m_hWnd);
	


	//auto [rcLauncher, rcToolBar, rcLeftFav, rcLeftTab, rcHSplitter, rcRightFav, rcRightTab, rcVSplitter, rcLog, rcStatus] = GetRects();

	m_spLauncher->OnCreate(CreateEvt(this, this, CRectF()));
	m_spToolBar->OnCreate(CreateEvt(this, this, CRectF()));
	auto spBtn = std::make_shared<CButton>(m_spToolBar.get());
	spBtn->Content.set(m_isPreview?L"Prv":L"Nrm");
	spBtn->Command.subscribe([this, spBtn]() {
		m_isPreview = !m_isPreview;
		spBtn->Content.set(m_isPreview ? L"Prv" : L"Nrm");
	}, shared_from_this());
	spBtn->OnCreate(CreateEvt(this, m_spToolBar.get(), CRectF()));

	//m_spLeftFavoritesView->OnCreate(CreateEvt(this, this, rcLeftFav));
	//m_spLeftView->OnCreate(CreateEvt(this, this, rcLeftTab));
	//m_spHSplitter->OnCreate(CreateEvt(this, this, rcHSplitter));
	//m_spVSplitter->OnCreate(CreateEvt(this, this, rcVSplitter));
	//m_spRightFavoritesView->OnCreate(CreateEvt(this, this, rcRightFav));
	//m_spRightView->OnCreate(CreateEvt(this, this, rcRightTab));
	m_spStatusBar->OnCreate(CreateEvt(this, this, CRectF()));
	//m_spLogText->OnCreate(CreateEvt(this, this, rcLog));
	//m_spBottomSplitContainer->OnCreate(CreateEvt(this, this, rcLog));

	m_spHorizontalSplit->OnCreate(CreateEvt(this, this, CRectF()));

	//MeasureArrange
	CRect rcClient = GetClientRect();
	//Measure
	Measure(rcClient.Size().Cast<CSizeF>());
	//Arrange
	Arrange(rcClient.Cast<CRectF>());

	//Update Log
	
	auto updateLog = [this] {
		std::wstring log;
		m_pPerformance->Update();
		PerformanceLog.set(m_pPerformance->OutputString());
		ThreadPoolLog.set(CThreadPool::GetInstance()->OutputString());
	};
	updateLog();
	m_timer.run([this, updateLog]()->void {
		GetDispatcherPtr()->PostInvoke(updateLog);
	}, std::chrono::milliseconds(3000));

	GetWndPtr()->SetFocusToControl(m_spHorizontalSplit->GetTop());
}

void CFilerWnd::OnKeyDown(const KeyDownEvent& e)
{
	// TODOTODO
	//*(e.HandledPtr) = FALSE;
	//switch (e.Char)
	//{
	//case 'Q':
	//	//Replace or PushBack
	//	if (::GetAsyncKeyState(VK_CONTROL)) {
	//		if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
	//			if (auto spCurView = std::dynamic_pointer_cast<CFilerView>(spCurTab->GetCurrentControlPtr())) {
	//				std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
	//				std::shared_ptr<TabData> spNewData;
	//				if (boost::iequals(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt(), L".txt")) {
	//					spNewData = std::make_shared<TextTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
	//				} else if (boost::iequals(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt(), L".pdf")) {
	//					spNewData = std::make_shared<PdfTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
	//				} else if (std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
	//					spNewData = std::make_shared<ImageTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
	//				} else if (std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
	//					spNewData = std::make_shared<PreviewTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
	//				}

	//				if (spNewData) {
	//					//Replace
	//					if (::IsKeyDown(VK_SHIFT) && spOtherTab->ItemsSource.get_unconst()->at(*spOtherTab->SelectedIndex)->AcceptClosing(this, false)) {
	//						spOtherTab->ItemsSource.replace(spOtherTab->ItemsSource.get_unconst()->begin() + *spOtherTab->SelectedIndex, spNewData);
	//					//Push back	
	//					} else {
	//						spOtherTab->ItemsSource.push_back(spNewData);
	//					}
	//					*(e.HandledPtr) = TRUE;
	//				}
	//			}
	//		}
	//	}
	//	break;

	//case VK_F4:
	//	{
	//		if (::GetAsyncKeyState(VK_MENU)) {
	//			OnCommandExit(CommandEvent(this, 0, 0, e.HandledPtr));
	//			*(e.HandledPtr) = TRUE;
	//		}
	//	}
	//	break;
	//case VK_F5:
	//	{
	//		if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
	//			if (auto spFilerView = std::dynamic_pointer_cast<CFilerView>(spCurTab->GetCurrentControlPtr())) {
	//				std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
	//				if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerView>(spOtherTab->GetCurrentControlPtr())) {
	//					spFilerView->GetFileGridPtr()->CopySelectedFilesTo(spOtherFilerGrid->GetFileGridPtr()->Folder->GetAbsoluteIdl());
	//					*(e.HandledPtr) = TRUE;
	//				}
	//			}
	//		}
	//	}
	//	break;
	//case VK_F6:
	//	{
	//		if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
	//			if (auto spFilerView = std::dynamic_pointer_cast<CFilerView>(spCurTab->GetCurrentControlPtr())) {
	//				std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
	//				if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerView>(spOtherTab->GetCurrentControlPtr())) {
	//					spFilerView->GetFileGridPtr()->MoveSelectedFilesTo(spOtherFilerGrid->GetFileGridPtr()->Folder->GetAbsoluteIdl());
	//					*(e.HandledPtr) = TRUE;
	//				}
	//			}
	//		}
	//	}
	//	break;
	//case VK_F9:
	//	{
	//		if (auto spCurTab = std::dynamic_pointer_cast<CFilerTabGridView>(GetFocusedControlPtr())) {
	//			if (auto spFilerView = std::dynamic_pointer_cast<CFilerView>(spCurTab->GetCurrentControlPtr())) {
	//				std::shared_ptr<CFilerTabGridView> spOtherTab = spCurTab == m_spLeftView ? m_spRightView : m_spLeftView;
	//				if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerView>(spOtherTab->GetCurrentControlPtr())) {
	//					spFilerView->GetFileGridPtr()->CopyIncrementalSelectedFilesTo(spOtherFilerGrid->GetFileGridPtr()->Folder->GetAbsoluteIdl());
	//					*(e.HandledPtr) = TRUE;
	//				}
	//			}
	//		}
	//	}
	//	break;
	//default:
	//	break;
	//}
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
	Rectangle.set(CRect(wp.rcNormalPosition));

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

//std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> CFilerWnd::GetRects()
//{
//	CRectF rcClient = GetRectInWnd();
//
//	LONG launcherHeight = GetDirectPtr()->Dips2PixelsX(
//		GetNormalBorder().Width * 2 + //def:GridLine=0.5*2, CellLine=0.5*2
//		GetPadding().left + //default:2
//		GetPadding().right + //default:2
//		16.f);//icon
//
//	LONG favoriteWidth = GetDirectPtr()->Dips2PixelsX(
//		GetNormalBorder().Width * 2 + //def:GridLine=0.5*2, CellLine=0.5*2
//		GetPadding().left + //default:2
//		GetPadding().right + //default:2
//		16.f);//icon
//	LONG statusHeight = GetDirectPtr()->Dips2PixelsY(m_spStatusBar->MeasureSize(L"").height);
//
//	CRectF rcLauncher, rcToolBar, rcLeftFavorites, rcRightFavorites, rcHSplitter, rcLeftGrid, rcRightGrid, rcVSplitter, rcLog, rcStatusBar;
//
//	rcLauncher.SetRect(rcClient.left, rcClient.top, (rcClient.left + rcClient.right)*0.5f, rcClient.top + launcherHeight);
//
//	m_spToolBar->Measure(CSizeF(FLT_MAX, FLT_MAX));
//	rcToolBar.SetRect((rcClient.left + rcClient.right)*0.5f, rcClient.top, rcClient.right , rcClient.top + launcherHeight);
//
//	rcStatusBar.SetRect(rcClient.left, rcClient.bottom - statusHeight, rcClient.right, rcClient.bottom);
//
//	m_spHSplitter->Minimum.set(rcClient.left);
//	m_spHSplitter->Maximum.set(rcClient.right - m_spHSplitter->DesiredSize().width);
//	if (*m_spHSplitter->Value < 0) {//Initial = No serialize
//		m_spHSplitter->Value.set((*m_spHSplitter->Maximum - *m_spHSplitter->Minimum)*0.5f);
//	}
//
//	m_spVSplitter->Minimum.set(rcClient.top + rcLauncher.Height());
//	m_spVSplitter->Maximum.set(rcClient.bottom - rcStatusBar.Height() - m_spVSplitter->DesiredSize().height);
//	if (*m_spVSplitter->Value == 0) {//Initial = No serialize
//		m_spVSplitter->Value.set((*m_spVSplitter->Maximum - *m_spVSplitter->Minimum)*0.5f);
//	}
//
//	FLOAT leftRight = *m_spHSplitter->Value;
//	FLOAT rightLeft = *m_spHSplitter->Value + m_spHSplitter->DesiredSize().width;
//
//	FLOAT topBottom = *m_spVSplitter->Value;
//	FLOAT bottomTop = *m_spVSplitter->Value + m_spVSplitter->DesiredSize().height;
//
//	rcLeftFavorites.SetRect(
//		rcClient.left, 
//		rcClient.top + launcherHeight,
//		rcClient.left + favoriteWidth,
//		topBottom);
//	rcLeftGrid.SetRect(
//		rcLeftFavorites.right,
//		rcClient.top + launcherHeight,
//		leftRight,
//		topBottom);
//	rcHSplitter.SetRect(
//		leftRight, 
//		rcClient.top + launcherHeight,
//		rightLeft,
//		topBottom);
//	rcRightFavorites.SetRect(
//		rightLeft,
//		rcClient.top  + launcherHeight,
//		rightLeft + favoriteWidth,
//		topBottom);
//	rcRightGrid.SetRect(
//		rcRightFavorites.right,
//		rcClient.top + launcherHeight,
//		rcClient.right,
//		topBottom
//	);
//	rcVSplitter.SetRect(
//		rcClient.left,
//		topBottom,
//		rcClient.right,
//		bottomTop
//	);
//	rcLog.SetRect(
//		rcClient.left,
//		bottomTop,
//		rcClient.right,
//		rcStatusBar.top
//	);
//
//	rcLeftGrid.DeflateRect(2.f);
//	rcRightGrid.DeflateRect(2.f);
//
//	return {
//		rcLauncher,
//		rcToolBar,
//		rcLeftFavorites,
//		rcLeftGrid,
//		rcHSplitter,
//		rcRightFavorites, 
//		rcRightGrid,
//		rcVSplitter,
//		rcLog,
//		rcStatusBar
//	};
//
//}

void CFilerWnd::Measure(const CSizeF& availableSize) 
{
	//GridView
	m_spLauncher->Measure(availableSize);
	//m_spLeftFavoritesView->Measure(availableSize);
	//m_spRightFavoritesView->Measure(availableSize);
	//ToolBar
	m_spToolBar->Measure(availableSize);
	//StatusBar
	m_spStatusBar->Measure(availableSize);

	m_spHorizontalSplit->Measure(availableSize);
	//Splitter
	//m_spHSplitter->Measure(availableSize);
	//m_spVSplitter->Measure(availableSize);

	//Auto
	//m_spLeftView->Measure(availableSize);
	//m_spRightView->Measure(availableSize);
	//m_spLogText->Measure(availableSize, L"A");
	//m_spBottomSplitContainer->Measure(availableSize);
}

void CFilerWnd::Arrange(const CRectF& rc)
{
	/*******/
	/* Top */
	/*******/
	//TopLeft
	m_spLauncher->Arrange(CRectF(
		rc.left,
		rc.top,
		(rc.left + rc.right) * 0.5f,
		rc.top + m_spLauncher->DesiredSize().height
	));
	//TopRight
	m_spToolBar->Arrange(CRectF(
		m_spLauncher->ArrangedRect().right,
		rc.top,
		rc.right,
		rc.top + m_spToolBar->DesiredSize().height
	));
	FLOAT topBottom = (std::max)(m_spLauncher->ArrangedRect().bottom, m_spToolBar->ArrangedRect().bottom);

	/**********/
	/* Bottom */
	/**********/
	m_spStatusBar->Arrange(CRectF(
		rc.left,
		rc.bottom - m_spStatusBar->DesiredSize().height,
		rc.right,
		rc.bottom
	));

	/********/
	/* Fill */
	/********/
	m_spHorizontalSplit->Arrange(CRectF(
		topBottom,
		rc.left,
		m_spStatusBar->ArrangedRect().top,
		rc.right));

	///********/
	///* Fill */
	///********/
	////Splitter
	//m_spHSplitter->Minimum.set(rc.left);
	//m_spHSplitter->Maximum.set(rc.right - m_spHSplitter->DesiredSize().width);
	//if (*m_spHSplitter->Value < 0) {//Initial = No serialize
	//	m_spHSplitter->Value.set((*m_spHSplitter->Maximum + *m_spHSplitter->Minimum)*0.5f);
	//}
	//m_spHSplitter->Value.set(std::clamp(*m_spHSplitter->Value, *m_spHSplitter->Minimum, *m_spHSplitter->Maximum));

	//m_spVSplitter->Minimum.set(topBottom);
	//m_spVSplitter->Maximum.set(m_spStatusBar->ArrangedRect().top - m_spVSplitter->DesiredSize().height);
	//if (*m_spVSplitter->Value < 0) {//Initial = No serialize
	//	m_spVSplitter->Value.set((*m_spVSplitter->Maximum + *m_spVSplitter->Minimum)*0.5f);
	//}
	//m_spVSplitter->Value.set(std::clamp(*m_spVSplitter->Value, *m_spVSplitter->Minimum, *m_spVSplitter->Maximum));

	//m_spHSplitter->Arrange(CRectF(
	//	*m_spHSplitter->Value, 
	//	*m_spVSplitter->Minimum,
	//	*m_spHSplitter->Value + m_spHSplitter->DesiredSize().width,
	//	*m_spVSplitter->Value));
	//m_spVSplitter->Arrange(CRectF(
	//	rc.left,
	//	*m_spVSplitter->Value,
	//	rc.right,
	//	*m_spVSplitter->Value + m_spVSplitter->DesiredSize().height
	//));
	////Fill Top
	//m_spLeftFavoritesView->Arrange(CRectF(
	//	rc.left, 
	//	topBottom,
	//	rc.left + m_spLeftFavoritesView->DesiredSize().width,
	//	m_spVSplitter->ArrangedRect().top));
	//m_spLeftView->Arrange(CRectF(
	//	m_spLeftFavoritesView->ArrangedRect().right,
	//	topBottom,
	//	m_spHSplitter->ArrangedRect().left,
	//	m_spVSplitter->ArrangedRect().top));
	//m_spRightFavoritesView->Arrange(CRectF(
	//	m_spHSplitter->ArrangedRect().right, 
	//	topBottom,
	//	m_spHSplitter->ArrangedRect().right + m_spRightFavoritesView->DesiredSize().width,
	//	m_spVSplitter->ArrangedRect().top));
	//m_spRightView->Arrange(CRectF(
	//	m_spRightFavoritesView->ArrangedRect().right,
	//	topBottom,
	//	rc.right,
	//	m_spVSplitter->ArrangedRect().top));
	////Fill Bottom
	//m_spBottomSplitContainer->Arrange(CRectF(
	//	rc.left,
	//	m_spVSplitter->ArrangedRect().bottom,
	//	rc.right,
	//	m_spStatusBar->ArrangedRect().top
	//)); 
	////m_spLogText->Arrange(CRectF(
	////	rc.left,
	////	m_spVSplitter->ArrangedRect().bottom,
	////	rc.right,
	////	m_spStatusBar->ArrangedRect().top
	////));
}
//TODOTODO CWnd m_pdirect, create, erasebkgnd, onsize
void CFilerWnd::OnRect(const RectEvent& e)
{
	//Measure
	Measure(e.Rect.Size());
	//Arrange
	Arrange(e.Rect);

	//auto [rcLauncher, rcToolBar, rcLeftFav, rcLeftTab, rcHSplitter, rcRightFav, rcRightTab, rcVSplitter, rcLog, rcStatus] = GetRects();
	
	//m_spLauncher->OnRect(RectEvent(this, rcLauncher));
	//m_spToolBar->Arrange(rcToolBar);
	//m_spLeftFavoritesView->OnRect(RectEvent(this, rcLeftFav));
	//m_spRightFavoritesView->OnRect(RectEvent(this, rcRightFav));
	//m_spStatusBar->OnRect(RectEvent(this, rcStatus));
	//m_spHSplitter->OnRect(RectEvent(this, rcHSplitter));
	//m_spVSplitter->OnRect(RectEvent(this, rcVSplitter));
	//m_spLeftView->OnRect(RectEvent(this, rcLeftTab));
	//m_spRightView->OnRect(RectEvent(this, rcRightTab));
	//m_spLogText->OnRect(RectEvent(this, rcLog));
}

//void CFilerWnd::OnSetFocus(const SetFocusEvent& e)
//{
//	if (!m_spCurView) { m_spCurView = m_spLeftView; }
//	if(m_spCurView && m_spCurView->GetContentWnd()){
//		::SetFocus(m_spCurView->GetContentWnd()->m_hWnd);
//	}
//}

void CFilerWnd::Reload()
{
	// TODOTODO
	//CDriveFolderManager::GetInstance()->Update();
	//m_spLauncher->Reload();
	////m_spLeftFavoritesView->Reload();
	////m_spRightFavoritesView->Reload();
	//m_spLeftView->GetFilerViewPtr()->GetFileGridPtr()->Reload();
	//m_spLeftView->GetFilerViewPtr()->GetFavoriteGridPtr()->Reload();
	//m_spRightView->GetFilerViewPtr()->GetFileGridPtr()->Reload();
	//m_spRightView->GetFilerViewPtr()->GetFavoriteGridPtr()->Reload();
	//InvalidateRect(NULL, FALSE);
}

LRESULT CFilerWnd::OnConnectivityChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_reloadTimer.run([this]()->void {
		GetDispatcherPtr()->PostInvoke([this] {
			::OutputDebugStringW(L"Reload\r\n");
			Reload();
		});
	}, std::chrono::milliseconds(2000));
	//m_reloadTimer.expires_from_now(boost::posix_time::milliseconds(30));
	//m_reloadTimer.async_wait([this](const boost::system::error_code& error)->void {

	//	if (error == boost::asio::error::operation_aborted) {
	//	} else {
	//		GetDispatcherPtr()->PostInvoke([this] {
	//			Reload();
	//		});
	//	}
	//});

	return 0;
}

LRESULT CFilerWnd::OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_spLeftView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	//m_spRightView->GetGridView()->GetDirectPtr()->GetIconCachePtr()->Clear();
	switch (wParam) {
		case DBT_DEVICEARRIVAL:
		case DBT_DEVICEREMOVECOMPLETE:
			m_reloadTimer.run([this]()->void {
				GetDispatcherPtr()->PostInvoke([this] {
					Reload();
				});
			}, std::chrono::milliseconds(2000));
			//m_reloadTimer.expires_from_now(boost::posix_time::milliseconds(30));
			//m_reloadTimer.async_wait([this](const boost::system::error_code& error)->void {

			//	if (error == boost::asio::error::operation_aborted) {
			//	} else {
			//		GetDispatcherPtr()->PostInvoke([this] {
			//			Reload();
			//		});
			//	}
			//});
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
	//TODOTODO
	//m_spFavoritesProp = CFilerApplication::GetInstance()->DeserializeFavoirtes();
	//m_spLeftView->GetFilerViewPtr()->GetFavoriteGridPtr()->ItemsSource = m_spFavoritesProp->Favorites;
	//m_spRightView->GetFilerViewPtr()->GetFavoriteGridPtr()->ItemsSource = m_spFavoritesProp->Favorites;

	//m_spLeftView->GetFilerViewPtr()->GetFavoriteGridPtr()->Reload();
	//m_spRightView->GetFilerViewPtr()->GetFavoriteGridPtr()->Reload();
	//InvalidateRect(NULL, FALSE);
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


