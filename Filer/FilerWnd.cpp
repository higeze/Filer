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

#include "DockPanel.h"
#include "ShellContextMenu.h"

#include "PreviewButton.h"

#include "ToDoTabData.h"
#include "FilerTabData.h"
#include "TextTabData.h"
#include "PDFTabData.h"
#include "ImageTabData.h"
#include "PreviewTabData.h"
#include "ThreadMonitorTabData.h"

#include "PDFPage.h"


std::vector<std::wstring> CFilerWnd::imageExts = { L".bmp", L".gif", L".ico", L".jpg", L".jpeg", L".png",L".tiff" };
std::vector<std::wstring> CFilerWnd::previewExts = {L".docx", L".doc", L".xlsx", L".xls", L".ppt", L".pptx"};

//Default
CFilerWnd::CFilerWnd()
	:CD2DWWindow(),
	//m_reloadIosv(), m_reloadWork(m_reloadIosv), m_reloadTimer(m_reloadIosv),
	Dummy(std::make_shared<int>(1)),
	Rectangle(0, 0, 1000, 600), 
	IsPreview(false),
	m_pPerformance(std::make_unique<CPerformance>()),
	//m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	//m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	//m_spLauncherProp(std::make_shared<CLauncherProperty>()),
	m_spExeExProp(std::make_shared<ExeExtensionProperty>())
	//spDock(std::make_shared<CDockPanel>(this))
	//m_spLauncher(std::make_shared<CLauncherGridView>(this, m_spLauncherProp)),
	//m_spToolBar(std::make_shared<CToolBar>(this)),
	//m_spHorizontalSplit(std::make_shared<CHorizontalSplitContainer>(this)),
	//m_spStatusBar(std::make_shared<CStatusBar>(this))
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

//void CFilerWnd::SetUpFilerGrid(const std::shared_ptr<CFilerTabGridView>& subject, const std::shared_ptr<CFilerTabGridView>& observer)
//{
//	subject->GetFilerViewPtr()->GetFileGridPtr()->StatusLog.subscribe(
//		[this](auto notify) {
//		m_spStatusBar->Text.set(notify.all_items);
//		InvalidateRect(NULL, FALSE);
//	}, shared_from_this());
//
//}

void CFilerWnd::SetUpPreview(const std::shared_ptr<CFilerTabGridView>& subject, const std::shared_ptr<CFilerTabGridView>& observer)
{
	subject->GetFilerViewPtr()->GetFileGridPtr()->SelectedItem.subscribe([this, wp = std::weak_ptr(observer)](const std::shared_ptr<CShellFile>& spFile) {

		if (auto observer = wp.lock()) {
			if (!*IsPreview) return;

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

	}, Dummy);
}

void CFilerWnd::SetUpFilerView(const std::shared_ptr<CFilerView>& view, const std::shared_ptr<CStatusBar>& status)
{
	Favorites.binding(view->GetFavoriteGridPtr()->ItemsSource);
	view->GetFileGridPtr()->StatusLog.subscribe(
		[this, wp = std::weak_ptr(status)](auto notify) {
		if (auto sp = wp.lock()) {
			sp->Text.set(notify.all_items);
			InvalidateRect(NULL, FALSE);
		}
	}, shared_from_this());

	view->SetIsTabStop(true);

	view->GetFileGridPtr()->GetFileContextMenu().Add(
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Add to Favorite",
		[this, wp = std::weak_ptr(view)]()->void {
			if (auto sp = wp.lock()) {
				auto files = sp->GetFileGridPtr()->GetSelectedFiles();
				for (auto& file : files) {
					Favorites.push_back(CFavorite(file->GetPath(), L""));//TODOMONITOR
				}
			}
		}),
		std::make_unique<CMenuItem2>(L"Add to Launcher",
		[this, wp = std::weak_ptr(view)]()->void {
			if (auto sp = wp.lock()) {
				auto files = sp->GetFileGridPtr()->GetSelectedFiles();
				for (auto& file : files) {
					Launchers.push_back(CLauncher(file->GetPath(), L""));//TODOMONITOR
				}
			}
		})
	);

	view->GetFileGridPtr()->GetFolderContextMenu().Add(
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Add to Favorite",
		[this, wp = std::weak_ptr(view)]()->void {
			if (auto sp = wp.lock()) {
				auto files = sp->GetFileGridPtr()->GetSelectedFiles();
				for (auto& file : files) {
					Favorites.push_back(CFavorite(file->GetPath(), L""));//TODOMONITO
				}
			}
		}));
}

void CFilerWnd::OnCreate(const CreateEvt& e)
{
	//Favorites = m_spFavoritesProp->Favorites;
	//Launchers = m_spLauncherProp->Launchers;

	//SetWindowPlacement make sure Window in Monitor
	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.rcNormalPosition = e.Rect;
	::SetWindowPlacement(m_hWnd, &wp);

	//Network Messanger
	m_pNetworkMessanger = std::make_unique<CNetworkMessanger>(m_hWnd);

	/************/
	/* Children */
	/************/
	if (m_childControls.empty()) {
		/*******/
		/* All */
		/*******/
		auto spDock = std::make_shared<CDockPanel>(this);
		using pr = std::pair<std::shared_ptr<CD2DWControl>, DockEnum>;

		/*******/
		/* Top */
		/*******/
		auto spTopDock = std::make_shared<CDockPanel>(spDock.get());
		//Launcher
		auto spLauncher = std::make_shared<CLauncherGridView>(spTopDock.get());
		//Tool bar
		auto spToolBar = std::make_shared<CToolBar>(spTopDock.get());
		auto spBtn = std::make_shared<CPreviewButton>(spToolBar.get());
		spToolBar->Add(spBtn);

		spTopDock->Add(
			pr(spToolBar, DockEnum::Right),
			pr(std::make_shared<CVerticalSplitter>(), DockEnum::Right),
			pr(spLauncher, DockEnum::Fill));

		/**********/
		/* Bottom */
		/**********/
		//Status bar
		auto spBottomStatus = std::make_shared<CStatusBar>(spDock.get());

		/********/
		/* Fill */
		/********/
		//Horizontal
		auto spFillDock = std::make_shared<CDockPanel>(spDock.get());

		//Horizontal Top
		auto spFillTopDock = std::make_shared<CDockPanel>(spFillDock.get());

		auto spLeftTop = std::make_shared<CFilerTabGridView>(spFillTopDock.get());
		auto spRightTop = std::make_shared<CFilerTabGridView>(spFillTopDock.get());

		spLeftTop->SetOther(spRightTop);
		spRightTop->SetOther(spLeftTop);
		spFillTopDock->Add(
			pr(spRightTop, DockEnum::Right),
			pr(std::make_shared<CVerticalSplitter>(), DockEnum::Right),
			pr(spLeftTop, DockEnum::Fill));

		//Horizontal Bottom
		auto spFillBottomDock = std::make_shared<CDockPanel>(spFillDock.get());

		//TODOHIGH Move to Tab
		//auto spTextBoxLeft = std::make_shared<CColoredTextBox>(spFillBottomDock.get(), L"");
		//PerformanceLog.binding(spTextBoxLeft->Text);
		//auto spTextBoxRight = std::make_shared<CColoredTextBox>(spFillBottomDock.get(), L"");
		//ThreadPoolLog.binding(spTextBoxRight->Text);
		//spFillBottomDock->Add(
		//	pr(spTextBoxRight, DockEnum::Right),
		//	pr(std::make_shared<CVerticalSplitter>(), DockEnum::Right),
		//	pr(spTextBoxLeft, DockEnum::Fill));

		auto spLeftBottom = std::make_shared<CFilerTabGridView>(spFillBottomDock.get());
		auto spRightBottom = std::make_shared<CFilerTabGridView>(spFillBottomDock.get());

		spLeftBottom->SetOther(spRightBottom);
		spRightBottom->SetOther(spLeftBottom);

		/********/
		/* Dock */
		/********/
		spFillBottomDock->Add(
			pr(spRightBottom, DockEnum::Right),
			pr(std::make_shared<CVerticalSplitter>(), DockEnum::Right),
			pr(spLeftBottom, DockEnum::Fill));


		spFillDock->Add(
			pr(spFillBottomDock, DockEnum::Bottom),
			pr(std::make_shared<CHorizontalSplitter>(), DockEnum::Bottom),
			pr(spFillTopDock, DockEnum::Fill));

		spDock->Add(
			pr(spTopDock, DockEnum::Top),
			pr(spBottomStatus, DockEnum::Bottom),
			pr(spFillDock, DockEnum::Fill));
	}

	//Create
	for (auto& child : m_childControls) {
		child->OnCreate(CreateEvt(this, this, CRectF()));
	}

	//Launchers
	if (Launchers->empty()) {
		Launchers.push_back(CLauncher(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT"));
	}

	//Favorites
	if (Favorites->empty()) {
		Favorites.push_back(CFavorite(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT"));
	}

	//FilerTabControl Connection
	std::vector<std::shared_ptr<CDockPanel>> docks = FindChildren<CDockPanel>(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	for (auto& dock : docks) {
		std::vector<std::shared_ptr<CFilerTabGridView>> tabs = FindChildren<CFilerTabGridView>(dock);
		if (tabs.size() == 2) {
			tabs[0]->SetOther(tabs[1]);
			tabs[1]->SetOther(tabs[0]);
		}
	}

	//Favorite Binding
	std::vector<std::shared_ptr<CFavoritesGridView>> favoriteviews = FindChildren<CFavoritesGridView>(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	for (auto& child : favoriteviews) {
		Favorites.binding(child->ItemsSource);
	}


	//Launcher Binding
	std::vector<std::shared_ptr<CLauncherGridView>> launchers = FindChildren<CLauncherGridView>(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	for (auto& child : launchers) {
		Launchers.binding(child->ItemsSource);
	}

	//Preview Binding
	std::vector<std::shared_ptr<CPreviewButton>> prvBtns = FindChildren<CPreviewButton>(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	for (auto& child : prvBtns) {
		IsPreview.binding(child->IsPreview);
	}
	
	//StatusBar
	std::vector<std::shared_ptr<CStatusBar>> statuses = FindChildren<CStatusBar>(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	
	//FilerView SetUp (Binding, etc)
	std::vector<std::shared_ptr<CFilerView>> views = FindChildren<CFilerView>(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	for (auto& child : views) {
		SetUpFilerView(child, statuses[0]);
	}

	//MeasureArrange
	CRect rcClient = GetClientRect();
	ProcessFunctionToAll([](auto child) { child->MeasureDirty.set(true); child->ArrangeDirty.set(true); });

	//Measure
	Measure(rcClient.Size().Cast<CSizeF>());
	//Arrange
	Arrange(rcClient.Cast<CRectF>());

	SetFocusToControl(m_childControls[0]);
}

void CFilerWnd::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char)
	{
	case VK_F4:
		{
			if (::GetAsyncKeyState(VK_MENU)) {
				OnCommandExit(CommandEvent(this, 0, 0, e.HandledPtr));
				*(e.HandledPtr) = TRUE;
			}
		}
		break;
	default:
		CD2DWWindow::OnKeyDown(e);
	}
}

void CFilerWnd::OnClose(const CloseEvent& e)
{	
	WINDOWPLACEMENT wp={0};
	wp.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	Rectangle.set(CRect(wp.rcNormalPosition));

	Close.execute();

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

CSizeF CFilerWnd::MeasureOverride(const CSizeF& availableSize) 
{
	for (auto child : m_childControls) {
		child->Measure(availableSize);
	}
	return availableSize;
}

void CFilerWnd::ArrangeOverride(const CRectF& finalRect)
{
	CD2DWWindow::ArrangeOverride(finalRect);

	if (!finalRect.IsRectNull()) {
		for (auto child : m_childControls) {
			child->Arrange(finalRect);
		}
	}
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
	// TODOMONITOR
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
	//TODOLOW
	//Launchers = CFilerApplication::GetInstance()->DeserializeLauncher();
	//Reload();
}


void CFilerWnd::OnCommandFavoritesOption(const CommandEvent& e)
{
	//TODOLOW
	//Favrotites = CFilerApplication::GetInstance()->DeserializeFavoirtes();
	//Reload();
}

void CFilerWnd::OnCommandExeExtensionOption(const CommandEvent& e)
{
	//TOODLOW
	//m_spExeExProp = CFilerApplication::GetInstance()->DeserializeExeExtension();
	//InvalidateRect(NULL, FALSE);
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


