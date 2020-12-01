#include "FilerTabGridView.h"
#include "FilerGridView.h"
#include "FilerGridViewProperty.h"
#include "ToDoGridView.h"
#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "FilerWnd.h"
#include "BindRow.h"
#include "KnownFolder.h"
#include "ShellFileFactory.h"
#include "Debug.h"

#include "BindRow.h"
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "BindCheckBoxColumn.h"
#include "BindCheckBoxCell.h"
#include "BindSheetCellColumn.h"
#include "BindItemsSheetCell.h"

#include "D2DWWindow.h"
#include "Dispatcher.h"
#include "Textbox.h"
#include "MyFile.h"
#include "FileIconCache.h"
#include "ShellFileFactory.h"
#include "PdfView.h"

FilerTabData::FilerTabData(const std::wstring& path)
	:TabData(), Path(path)
{
	if (!Path.empty()) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
		if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
			FolderPtr = sp;
		} else {
			FolderPtr = CKnownFolderManager::GetInstance()->GetDesktopFolder();
			Path = FolderPtr->GetPath();
		}
	}
}


void TextTabData::Open()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//ofn.lpstrDefExt = L"txt";

	if (!GetOpenFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			throw std::exception(FILE_LINE_FUNC);
		}
	} else {
		::ReleaseBuffer(path);
		Open(path);
	}
}

void TextTabData::Open(const std::wstring& path)
{
	if (::PathFileExists(path.c_str())) {
		Path.set(path);
		Text.assign(str2wstr(CFile::ReadAllString<char>(path)));
		Status.force_notify_set(TextStatus::Saved);
		Carets.set(0, 0, 0, 0, 0);
		CaretPos.set(CPointF(0, 10 * 0.5f));//TODOLOW
	} else {
		Path.set(L"");
		Status.force_notify_set(TextStatus::Saved);
		Carets.set(0, 0, 0, 0, 0);
		CaretPos.set(CPointF(0, 10 * 0.5f));//TODOLOW
	}
}

void TextTabData::Save()
{
	if (!::PathFileExistsW(Path.c_str())) {
		std::wstring path;
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
		ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
		ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = L"Save as";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = L"txt";

		if (!GetSaveFileName(&ofn)) {
			DWORD errCode = CommDlgExtendedError();
			if (errCode) {
				//throw std::exception(FILE_LINE_FUNC);
			}
		} else {
			::ReleaseBuffer(path);
		}
		Save(path);
	} else {
		Save(Path);
	}
}

void TextTabData::Save(const std::wstring& path)
{
	Path.set(path);
	Status.force_notify_set(TextStatus::Saved);
	CFile::WriteAllString(path, wstr2str(Text));
}

CFilerTabGridView::CFilerTabGridView(CD2DWControl* pParentControl, 
	std::shared_ptr<TabControlProperty> spTabProp,
	std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp, 
	std::shared_ptr<TextEditorProperty>& spTextEditorProp,
	std::shared_ptr<PdfViewProperty>& spPdfViewProp)
	:CTabControl(pParentControl, spTabProp), 
	m_spFilerGridViewProp(spFilerGridViewProp),
	m_spTextEditorProp(spTextEditorProp),
	m_spPdfViewProp(spPdfViewProp)
{
	//Command
	m_commandMap.emplace(IDM_NEWFILERTAB, std::bind(&CFilerTabGridView::OnCommandNewFilerTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWTODOTAB, std::bind(&CFilerTabGridView::OnCommandNewToDoTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWTEXTTAB, std::bind(&CFilerTabGridView::OnCommandNewTextTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWPDFTAB, std::bind(&CFilerTabGridView::OnCommandNewPdfTab, this, phs::_1));

	m_commandMap.emplace(IDM_CLONETAB, std::bind(&CFilerTabGridView::OnCommandCloneTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLOSETAB, std::bind(&CFilerTabGridView::OnCommandCloseTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLOSEALLBUTTHISTAB, std::bind(&CFilerTabGridView::OnCommandCloseAllButThisTab, this, phs::_1));
	m_commandMap.emplace(IDM_ADDTOFAVORITE, std::bind(&CFilerTabGridView::OnCommandAddToFavorite, this, phs::_1));
	m_commandMap.emplace(IDM_OPENSAMEASOTHER, std::bind(&CFilerTabGridView::OnCommandOpenSameAsOther, this, phs::_1));

	//ItemsHeader
	m_itemsHeaderTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{

			if (auto p = std::dynamic_pointer_cast<FilerTabData>(pTabData)) {
				return p->FolderPtr->GetFileNameWithoutExt().c_str();
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<ToDoTabData>(pTabData)) {
				return ::PathFindFileName(p->Path.c_str());
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				return std::wstring(p->Status.get() == TextStatus::Dirty?L"*":L"") + ::PathFindFileName(p->Path.c_str());
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				return std::wstring(::PathFindFileName(p->Path.c_str()));
			} else {
				return L"nullptr";
			}
		});

	//ItemsHeaderIcon
	std::function<void()> updated = [this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]()->void
		{
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		});
	};

	m_itemsHeaderIconTemplate.emplace(typeid(FilerTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{

			if (auto p = std::dynamic_pointer_cast<FilerTabData>(pTabData)) {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetFileIconBitmap(
					p->FolderPtr->GetAbsoluteIdl(), p->FolderPtr->GetPath(), p->FolderPtr->GetDispExt(), updated);
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(ToDoTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{
			if (auto p = std::dynamic_pointer_cast<ToDoTabData>(pTabData)) {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(TextTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Path);
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetFileIconBitmap(
					spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), updated);
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(PdfTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Path);
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetFileIconBitmap(
					spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), updated);
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});

	//ItemsTemplate
	m_itemsControlTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto pData = std::static_pointer_cast<FilerTabData>(pTabData);
		auto spView = GetFilerGridViewPtr();
		spView->OpenFolder(pData->FolderPtr);
		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto pData = std::static_pointer_cast<ToDoTabData>(pTabData);
		auto spView = GetToDoGridViewPtr();
		spView->Open(pData->Path);
		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<TextTabData>(pTabData);
		auto spView = GetTextViewPtr();

		//Path
		m_pTextPathBinding.reset(nullptr);//Need to dispose first to disconnect
		m_pTextPathBinding.reset(new CBinding<std::wstring>(spViewModel->Path, spView->GetPath()));
		m_pTextPathConnection = std::make_unique<sigslot::scoped_connection>(spView->GetPath().Subscribe([this](const auto&) { GetHeaderRects().clear(); }));
		//Status
		m_pStatusBinding.reset(nullptr);
		m_pStatusConnection = std::make_unique<sigslot::scoped_connection>(spViewModel->Status.Subscribe([this](const auto&) { GetHeaderRects().clear(); }));
		//Text
		m_pTextBinding.reset(nullptr);
		m_pTextBinding.reset(new CBinding<std::wstring>(spViewModel->Text, spView->GetText()));
		//Carets
		m_pCaretsBinding.reset(nullptr);
		m_pCaretsBinding.reset(new CBinding<std::tuple<int, int, int, int, int>>(spViewModel->Carets, spView->GetCarets()));
		//CaretPos
		m_pCaretPosBinding.reset(nullptr);
		m_pCaretPosBinding.reset(new CBinding<CPointF>(spViewModel->CaretPos, spView->GetCaretPos()));
		//Open
		m_pOpenBinding.reset(nullptr);
		m_pOpenBinding.reset(new CBinding<void>(spViewModel->OpenCommand, spView->GetOpenCommand()));
		//Save()
		m_pSaveBinding.reset(nullptr);
		m_pSaveBinding.reset(new CBinding<void>(spViewModel->SaveCommand, spView->GetSaveCommand()));
		//Close
		spViewModel->ClosingFunction = [wp = std::weak_ptr(spViewModel), hWnd = GetWndPtr()->m_hWnd]()->bool
		{
			if (auto sp = wp.lock()) {
				if (sp->Status.get() == TextStatus::Dirty) {
					int ync = MessageBoxW(
						hWnd,
						fmt::format(L"\"{}\" is not saved.\r\nDo you like to save?", ::PathFindFileName(sp->Path.c_str())).c_str(),
						L"Save?",
						MB_YESNOCANCEL);
					switch (ync) {
						case IDYES:
							sp->Save();
							return true;
						case IDNO:
							return true;
						case IDCANCEL:
							return false;
						default:
							return true;
					}
				} else {
					return true;
				}
			}
			return true;
		};


		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		if (spViewModel->Status.get() == TextStatus::None) {
			spViewModel->Open(spViewModel->Path);
		}

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PdfTabData>(pTabData);
		auto spView = GetPdfViewPtr();

		//Path
		m_pPdfPathBinding.reset(nullptr);//Need to dispose first to disconnect
		m_pPdfPathBinding.reset(new CBinding<std::wstring>(spViewModel->Path, spView->GetPath()));
		m_pPdfPathConnection = std::make_unique<sigslot::scoped_connection>(spView->GetPath().Subscribe([this](const auto&) { GetHeaderRects().clear(); }));
		//Open
		//m_pOpenBinding.reset(nullptr);
		//m_pOpenBinding.reset(new CBinding<void>(spViewModel->OpenCommand, spView->GetOpenCommand()));

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		spView->Open(spViewModel->Path);//TODOHIGH

		return spView;
	});



	//FilerGridView Closure
	GetFilerGridViewPtr =
		[spFilerView = std::make_shared<CFilerGridView>(this, m_spFilerGridViewProp), isInitialized = false, this]()mutable->std::shared_ptr<CFilerGridView> {

		if (!isInitialized && GetWndPtr()->IsWindow()) {
			isInitialized = true;

			//Folder Changed
			spFilerView->FolderChanged = [&](std::shared_ptr<CShellFolder>& pFolder) {
				auto pData = std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex.get()]);
				pData->FolderPtr = pFolder;
				pData->Path = pFolder->GetPath();
				GetHeaderRects().clear();
				spFilerView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
			};
		}
		return spFilerView;
	};

	//ToDoGridView Closure
	GetToDoGridViewPtr = [spToDoView = std::make_shared<CToDoGridView>(this, std::static_pointer_cast<GridViewProperty>(m_spFilerGridViewProp)), isInitialized = false, this]()mutable->std::shared_ptr<CToDoGridView>{

		if (!isInitialized && GetWndPtr()->IsWindow()) {
			isInitialized = true;
			//Columns
			spToDoView->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(spToDoView.get()));
			spToDoView->PushColumns(
				spToDoView->GetHeaderColumnPtr(),
				std::make_shared<CBindCheckBoxColumn<MainTask>>(
					spToDoView.get(),
					L"Done",
					[](const std::tuple<MainTask>& tk)->CheckBoxState {return std::get<MainTask>(tk).Done ? CheckBoxState::True : CheckBoxState::False; },
					[](std::tuple<MainTask>& tk, const CheckBoxState& state)->void {std::get<MainTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
				std::make_shared<CBindTextColumn<MainTask>>(
					spToDoView.get(),
					L"Name",
					[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Name; },
					[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Name = str; }),
				std::make_shared<CBindTextColumn<MainTask>>(
					spToDoView.get(),
					L"Memo",
					[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Memo; },
					[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Memo = str; }),
				std::make_shared<CBindSheetCellColumn< MainTask, SubTask>>(
					spToDoView.get(),
					L"Sub Task",
					[](std::tuple<MainTask>& tk)->ReactiveVectorProperty<std::tuple<SubTask>>& {return std::get<MainTask>(tk).SubTasks; },
					[](CBindItemsSheetCell<MainTask, SubTask>* pCell)->void {
						pCell->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(pCell));
						pCell->PushColumns(
							pCell->GetHeaderColumnPtr(),
							std::make_shared<CBindCheckBoxColumn<SubTask>>(
								pCell,
								L"Done",
								[](const std::tuple<SubTask>& tk)->CheckBoxState {return std::get<SubTask>(tk).Done ? CheckBoxState::True : CheckBoxState::False; },
								[](std::tuple<SubTask>& tk, const CheckBoxState& state)->void {std::get<SubTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
							std::make_shared<CBindTextColumn<SubTask>>(
								pCell,
								L"Name",
								[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Name; },
								[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Name = str; }),
							std::make_shared<CBindTextColumn<SubTask>>(
								pCell,
								L"Memo",
								[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Memo; },
								[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Memo = str; })
						);
						pCell->SetFrozenCount<ColTag>(1);

						pCell->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(pCell));
						pCell->InsertRow(0, pCell->GetNameHeaderRowPtr());
						pCell->SetFrozenCount<RowTag>(1);
					},
					arg<"maxwidth"_s>() = FLT_MAX)
			);
			spToDoView->SetFrozenCount<ColTag>(1);

			//Rows
			spToDoView->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(spToDoView.get()));
			spToDoView->SetFilterRowPtr(std::make_shared<CRow>(spToDoView.get()));

			spToDoView->PushRows(
				spToDoView->GetNameHeaderRowPtr(),
				spToDoView->GetFilterRowPtr());

			spToDoView->SetFrozenCount<RowTag>(2);

			//Path Changed
			spToDoView->GetPath().Subscribe([&](const std::wstring& e) {
				auto pData = std::static_pointer_cast<ToDoTabData>(m_itemsSource[m_selectedIndex.get()]);
				pData->Path = spToDoView->GetPath().get();
				spToDoView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
			});
		}
		return spToDoView;
	};

	//TextView Closure
	GetTextViewPtr = [spTextView = std::make_shared<CTextEditor>(this, m_spTextEditorProp), isInitialized = false, this]()mutable->std::shared_ptr<CTextEditor>{
		return spTextView;
	};

	//PdfView Closure
	GetPdfViewPtr = [spPdfView = std::make_shared<CPdfView>(this, m_spPdfViewProp), isInitialized = false, this]()mutable->std::shared_ptr<CPdfView>{
		return spPdfView;
	};
}

CFilerTabGridView::~CFilerTabGridView() = default;

void CFilerTabGridView::OnCreate(const CreateEvt& e)
{
	CTabControl::OnCreate(e);
	GetFilerGridViewPtr()->OnCreate(CreateEvt(GetWndPtr(), GetControlRect()));
	GetToDoGridViewPtr()->OnCreate(CreateEvt(GetWndPtr(), GetControlRect()));
	GetTextViewPtr()->OnCreate(CreateEvt(GetWndPtr(), GetControlRect()));
	GetPdfViewPtr()->OnCreate(CreateEvt(GetWndPtr(), GetControlRect()));

	//ItemsSource
	if (m_itemsSource.empty()) {
		m_itemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
		m_itemsSource.push_back(std::make_shared<ToDoTabData>(L""));
		m_itemsSource.push_back(std::make_shared<TextTabData>(L""));
		m_selectedIndex.set(0);
	} else {
		if (m_selectedIndex.get() < 0) {
			m_selectedIndex.force_notify_set(0); 
		} else {
			m_selectedIndex.force_notify_set(m_selectedIndex.get());
		}
	}
}

void CFilerTabGridView::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char)
	{
	case 'T':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			GetWndPtr()->SendMessage(WM_COMMAND, IDM_NEWFILERTAB, NULL);
		}
		break;
	case 'W':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			GetWndPtr()->SendMessage(WM_COMMAND, IDM_CLOSETAB, NULL);
		}
		break;
	default:
		m_spCurControl->OnKeyDown(e);
		break;
	}
}


void CFilerTabGridView::OnContextMenu(const ContextMenuEvent& e)
{
	CTabControl::OnContextMenu(e);

	if (m_contextIndex) {
		CMenu menu;
		menu.Attach(::GetSubMenu(::LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CONTEXTMENU_TAB)), 0));
		GetWndPtr()->SetForegroundWindow();
		menu.TrackPopupMenu(0, e.PointInScreen.x, e.PointInScreen.y, GetWndPtr()->m_hWnd);
	} else if (m_spCurControl->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient))){
		m_spCurControl->OnContextMenu(e);
	}
}

void CFilerTabGridView::OnCommandNewFilerTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewToDoTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<ToDoTabData>(L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewTextTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<TextTabData>(L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewPdfTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<PdfTabData>(L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandAddToFavorite(const CommandEvent& e)
{
	//TODOLOW Bad connection between FilerTabGridView and FavoritesView
	if(auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())){
		p->GetFavoritesPropPtr()->GetFavorites().push_back(std::make_shared<CFavorite>(std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex])->Path, L""));
		p->GetLeftFavoritesView()->SubmitUpdate();
		p->GetRightFavoritesView()->SubmitUpdate();
	}
}

void CFilerTabGridView::OnCommandOpenSameAsOther(const CommandEvent& e)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if (auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())) {
			
		std::shared_ptr<CFilerTabGridView> otherView = (this == p->GetLeftWnd().get())? p->GetRightWnd(): p->GetLeftWnd();
		
		if (m_itemsSource[m_selectedIndex.get()]->ClosingFunction()) {
		m_itemsSource.replace(m_itemsSource.begin() + m_selectedIndex.get(), otherView->GetItemsSource()[otherView->GetSelectedIndex()]);
		}
	}
}




