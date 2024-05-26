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
#include "ShellContextMenu.h"


std::vector<std::wstring> CFilerWnd::imageExts = { L".bmp", L".gif", L".ico", L".jpg", L".jpeg", L".png",L".tiff" };
std::vector<std::wstring> CFilerWnd::previewExts = {L".docx", L".doc", L".xlsx", L".xls", L".ppt", L".pptx"};

//Default
CFilerWnd::CFilerWnd()
	:CD2DWWindow(),
	//m_reloadIosv(), m_reloadWork(m_reloadIosv), m_reloadTimer(m_reloadIosv),
	Dummy(std::make_shared<int>(1)),
	Rectangle(0, 0, 1000, 600), 
	m_pPerformance(std::make_unique<CPerformance>()),
	m_spApplicationProp(std::make_shared<CApplicationProperty>()),
	m_spFavoritesProp(std::make_shared<CFavoritesProperty>()),
	m_spLauncherProp(std::make_shared<CLauncherProperty>()),
	m_spExeExProp(std::make_shared<ExeExtensionProperty>()),
	m_spDock(std::make_shared<CDockPanel>(this))
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

	}, Dummy);
}

std::shared_ptr<CFilerTabGridView> CFilerWnd::CreateFilerTab(const std::shared_ptr<CD2DWControl>& parent, const std::shared_ptr<CStatusBar>& status)
{
	auto spTab = std::make_shared<CFilerTabGridView>(parent.get());
	Favorites.binding(spTab->GetFilerViewPtr()->GetFavoriteGridPtr()->ItemsSource);
	spTab->GetFilerViewPtr()->GetFileGridPtr()->StatusLog.subscribe(
		[this, status](auto notify) {
		status->Text.set(notify.all_items);
		InvalidateRect(NULL, FALSE);
	}, shared_from_this());

	spTab->SetIsTabStop(true);

	spTab->GetFilerViewPtr()->GetFileGridPtr()->GetFolderContextMenu().Add(
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Add to Favorite", [this, spTab]()->void {
		auto files = spTab->GetFilerViewPtr()->GetFileGridPtr()->GetSelectedFiles();
		for (auto& file : files) {
			Favorites.push_back(CFavorite(file->GetPath(), L""));//TODOTODO
		}}),
		std::make_unique<CMenuItem2>(L"Add to Launcher", [this, spTab]()->void {
			auto files = spTab->GetFilerViewPtr()->GetFileGridPtr()->GetSelectedFiles();
			for (auto& file : files) {
				Launchers.push_back(CLauncher(file->GetPath(), L""));//TODOTODO
			}
		})
	);
	return spTab;

}

void CFilerWnd::OnCreate(const CreateEvt& e)
{
	Favorites = m_spFavoritesProp->Favorites;
	Launchers = m_spLauncherProp->Launchers;

	//SetWindowPlacement make sure Window in Monitor
	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.rcNormalPosition = e.Rect;
	::SetWindowPlacement(m_hWnd, &wp);

	//Network Messanger
	m_pNetworkMessanger = std::make_unique<CNetworkMessanger>(m_hWnd);

	//Control Set up
	/*******/
	/* Top */
	/*******/
	auto spTopDock = std::make_shared<CDockPanel>(m_spDock.get());
	spTopDock->Dock.set(DockEnum::TopFix);
	//Launcher
	auto spLauncher = std::make_shared<CLauncherGridView>(spTopDock.get());
	spLauncher->Dock.set(DockEnum::Fill);
	//Tool bar
	auto spToolBar = std::make_shared<CToolBar>(spTopDock.get());
	spToolBar->Dock.set(DockEnum::Right);
	auto spBtn = std::make_shared<CButton>(spToolBar.get());
	spBtn->Content.set(m_isPreview?L"Prv":L"Nrm");
	spBtn->Command.subscribe([this, spBtn]() {
		m_isPreview = !m_isPreview;
		spBtn->Content.set(m_isPreview ? L"Prv" : L"Nrm");
	}, shared_from_this());
	spBtn->OnCreate(CreateEvt(this, spToolBar.get(), CRectF()));

	spTopDock->Add(spToolBar, spLauncher);

	/**********/
	/* Bottom */
	/**********/
	//Status bar
	auto spBottomStatus = std::make_shared<CStatusBar>(m_spDock.get());
	spBottomStatus->Dock.set(DockEnum::BottomFix);

	/********/
	/* Fill */
	/********/
	//Horizontal
	auto spFillDock = std::make_shared<CDockPanel>(m_spDock.get());
	spFillDock->Dock.set(DockEnum::Fill);

	//Horizontal Top
	auto spFillTopDock = std::make_shared<CDockPanel>(spFillDock.get());
	spFillTopDock->Dock.set(DockEnum::Fill);

	auto spLeftTop = CreateFilerTab(spFillTopDock, spBottomStatus);
	spLeftTop->Dock.set(DockEnum::Fill);

	auto spRightTop = CreateFilerTab(spFillTopDock, spBottomStatus);
	spRightTop->Dock.set(DockEnum::Right);

	spLeftTop->SetOther(spRightTop);
	spRightTop->SetOther(spLeftTop);
	spFillTopDock->Add(spRightTop, spLeftTop);

	//Horizontal Bottom
	auto spFillBottomDock = std::make_shared<CDockPanel>(spFillDock.get());
	spFillBottomDock->Dock.set(DockEnum::Bottom);

	auto spTextBoxLeft = std::make_shared<CColoredTextBox>(spFillBottomDock.get(), L"");
	PerformanceLog.binding(spTextBoxLeft->Text);
	spTextBoxLeft->Dock.set(DockEnum::Fill);

	auto spTextBoxRight = std::make_shared<CColoredTextBox>(spFillBottomDock.get(), L"");
	ThreadPoolLog.binding(spTextBoxRight->Text);
	spTextBoxRight->Dock.set(DockEnum::Right);

	spFillBottomDock->Add(spTextBoxRight, spTextBoxLeft);

	spFillDock->Add(spFillBottomDock, spFillTopDock);

	m_spDock->Add(spTopDock, spBottomStatus, spFillDock);

	m_spDock->OnCreate(CreateEvt(this, this, CRectF()));

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

	SetFocusToControl(spTopDock);
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

void CFilerWnd::Measure(const CSizeF& availableSize) 
{
	m_spDock->Measure(availableSize);
}

void CFilerWnd::Arrange(const CRectF& rc)
{
	m_spDock->Arrange(rc);
}

void CFilerWnd::OnRect(const RectEvent& e)
{
	//Measure
	Measure(e.Rect.Size());
	//Arrange
	Arrange(e.Rect);
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
	//Launchers = CFilerApplication::GetInstance()->DeserializeLauncher();
	//Reload();
}


void CFilerWnd::OnCommandFavoritesOption(const CommandEvent& e)
{
	//Favrotites = CFilerApplication::GetInstance()->DeserializeFavoirtes();
	//Reload();
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


