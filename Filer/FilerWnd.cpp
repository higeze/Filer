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

void CFilerWnd::OnCreate(const CreateEvt& e)
{
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
	auto spLauncher = std::make_shared<CLauncherGridView>(spTopDock.get(), m_spLauncherProp);
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
	auto spFillHorizontal = std::make_shared<CDockPanel>(m_spDock.get());
	spFillHorizontal->Dock.set(DockEnum::Fill);

	//Horizontal Top
	auto spTopVerticalSplit = std::make_shared<CDockPanel>(spFillHorizontal.get());
	spTopVerticalSplit->Dock.set(DockEnum::Fill);

	auto spTabLeft = std::make_shared<CFilerTabGridView>(spTopVerticalSplit.get());
	spTabLeft->GetFilerViewPtr()->GetFavoriteGridPtr()->ItemsSource.set(*m_spFavoritesProp->Favorites);
	spTabLeft->GetFilerViewPtr()->GetFileGridPtr()->StatusLog.subscribe(
		[this, spBottomStatus](auto notify) {
		spBottomStatus->Text.set(notify.all_items);
		InvalidateRect(NULL, FALSE);
	}, shared_from_this());	spTabLeft->SetIsTabStop(true);
	spTabLeft->Dock.set(DockEnum::Fill);


	auto spTabRight = std::make_shared<CFilerTabGridView>(spTopVerticalSplit.get());
	spTabRight->GetFilerViewPtr()->GetFavoriteGridPtr()->ItemsSource.set(*m_spFavoritesProp->Favorites);
	spTabRight->GetFilerViewPtr()->GetFileGridPtr()->StatusLog.subscribe(
		[this, spBottomStatus](auto notify) {
		spBottomStatus->Text.set(notify.all_items);
		InvalidateRect(NULL, FALSE);
	}, shared_from_this());
	spTabRight->SetIsTabStop(true);
	spTabRight->Dock.set(DockEnum::Right);

	SetUpPreview(spTabLeft, spTabRight);
	SetUpPreview(spTabRight, spTabLeft);

	spTopVerticalSplit->Add(spTabRight, spTabLeft);

	//Horizontal Bottom
	auto spBottomVerticalSplit = std::make_shared<CDockPanel>(spFillHorizontal.get());
	spBottomVerticalSplit->Dock.set(DockEnum::Bottom);

	auto spTextBoxLeft = std::make_shared<CColoredTextBox>(spBottomVerticalSplit.get(), L"");
	PerformanceLog.binding(spTextBoxLeft->Text);
	spTextBoxLeft->Dock.set(DockEnum::Fill);

	auto spTextBoxRight = std::make_shared<CColoredTextBox>(spBottomVerticalSplit.get(), L"");
	ThreadPoolLog.binding(spTextBoxRight->Text);
	spTextBoxRight->Dock.set(DockEnum::Right);

	spBottomVerticalSplit->Add(spTextBoxRight, spTextBoxLeft);

	spFillHorizontal->Add(spBottomVerticalSplit, spTopVerticalSplit);

	m_spDock->Add(spTopDock, spBottomStatus, spFillHorizontal);

	m_spDock->OnCreate(CreateEvt(this, this, CRectF()));

	//Context Menu for FileView
	//TODOTODO
	//CShellContextMenu menu;
	//menu.Add(
	//	std::make_unique<CMenuSeparator2>(),
	//	std::make_unique<CMenuItem2>(L"Add to Favorite", [this, files]()->void {
	//		for (auto& file : files) {
	//			GetFavoritesPropPtr()->Favorites.push_back(std::make_shared<CFavorite>(file->GetPath(), L""));
	//		}
	//		// TODOTODO
	//		//m_spLeftView->GetFilerViewPtr()->GetFavoriteGridPtr()->SubmitUpdate();
	//		//m_spLeftView->GetFilerViewPtr()->GetFavoriteGridPtr()->SubmitUpdate();
	//		//m_spLeftFavoritesView->SubmitUpdate();
	//		//m_spRightFavoritesView->SubmitUpdate();
	//}),
	//	std::make_unique<CMenuItem2>(L"Add to Launcher", [this, pWnd = GetWndPtr(), files]()->void {
	//		for (auto& file : files) {
	//			GetLauncherPropPtr()->Launchers.push_back(std::make_shared<CLauncher>(file->GetPath(), L""));
	//		}
	//		m_spLauncher->SubmitUpdate();
	//	})
	//);

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

	//TODOTODO
	//GetWndPtr()->SetFocusToControl(spHorizontalSplit->GetTop());
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
	//TODOTODO
	//m_spLauncherProp = CFilerApplication::GetInstance()->DeserializeLauncher();
	//m_spLauncher->ItemsSource = m_spLauncherProp->Launchers;

	//m_spLauncher->Reload();
	//InvalidateRect(NULL, FALSE);
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


