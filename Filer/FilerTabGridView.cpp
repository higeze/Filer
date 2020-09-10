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

#include "TextboxWnd.h"
#include "Textbox.h"


CFilerTabGridView::CFilerTabGridView(CWnd* pWnd, std::shared_ptr<TabControlProperty> spTabProp, std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp, std::shared_ptr<TextEditorProperty>& spTextEditorProp)
	:CTabControl(pWnd, spTabProp), m_spFilerGridViewProp(spFilerGridViewProp), m_spTextEditorProp(spTextEditorProp)
{
	//
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
				return std::wstring(p->IsSaved?L"":L"*") + ::PathFindFileName(p->Path.c_str());
			} else {
				return L"nullptr";
			}
		});

	//ItemsTemplate
	m_itemsControlTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CUIElement> {
		auto pData = std::static_pointer_cast<FilerTabData>(pTabData);
		auto spView = GetFilerGridViewPtr();
		spView->OpenFolder(pData->FolderPtr);
		spView->UpdateAll();
		spView->SetUpdateRect(GetControlRect());
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CUIElement> {
		auto pData = std::static_pointer_cast<ToDoTabData>(pTabData);
		auto spView = GetToDoGridViewPtr();
		spView->Open(pData->Path);
		spView->UpdateAll();
		spView->SetUpdateRect(GetControlRect());
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CUIElement> {
		auto pData = std::static_pointer_cast<TextTabData>(pTabData);
		auto spView = GetTextViewPtr();
		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		spView->Open(pData->Path);

		return spView;
	});


	//FilerGridView Closure
	GetFilerGridViewPtr =
		[spFilerView = std::make_shared<CFilerGridView>(pWnd, m_spFilerGridViewProp), isInitialized = false, this]()mutable->std::shared_ptr<CFilerGridView> {

		if (!isInitialized && GetWndPtr()->IsWindow()) {
			isInitialized = true;

			//Folder Changed
			spFilerView->FolderChanged = [&](std::shared_ptr<CShellFolder>& pFolder) {
				auto pData = std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex.get()]);
				pData->FolderPtr = pFolder;
				pData->Path = pFolder->GetPath();
				spFilerView->PostUpdate(Updates::Rect);
				spFilerView->SetUpdateRect(GetControlRect());
			};
			//Create
			spFilerView->OnCreate(CreateEvent(GetWndPtr(), NULL, NULL, nullptr));
		}

		return spFilerView;
	};

	//ToDoGridView Closure
	GetToDoGridViewPtr = [spToDoView = std::make_shared<CToDoGridView>(GetWndPtr(), std::static_pointer_cast<GridViewProperty>(m_spFilerGridViewProp)), isInitialized = false, this]()mutable->std::shared_ptr<CToDoGridView>{

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
					[](std::tuple<MainTask>& tk)->observable_vector<std::tuple<SubTask>>& {return std::get<MainTask>(tk).SubTasks; },
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
			spToDoView->GetObsPath().Changed.connect([&](const NotifyChangedEventArgs<std::wstring>& e) {
				auto pData = std::static_pointer_cast<ToDoTabData>(m_itemsSource[m_selectedIndex.get()]);
				pData->Path = spToDoView->GetObsPath().get();
				spToDoView->PostUpdate(Updates::Rect);
				spToDoView->SetUpdateRect(GetControlRect());
			});

			spToDoView->OnCreate(CreateEvent(GetWndPtr(), NULL, NULL, nullptr));
		}
		return spToDoView;
	};


	//TOTODODO
	//AddCmdIDHandler(IDM_NEWFILERTAB, &CFilerTabGridView::OnCommandNewFilerTab, this);
	//AddCmdIDHandler(IDM_NEWTODOTAB, &CFilerTabGridView::OnCommandNewToDoTab, this);
	//AddCmdIDHandler(IDM_NEWTEXTTAB, &CFilerTabGridView::OnCommandNewTextTab, this);

	//AddCmdIDHandler(IDM_CLONETAB, &CFilerTabGridView::OnCommandCloneTab, this);
	//AddCmdIDHandler(IDM_CLOSETAB, &CFilerTabGridView::OnCommandCloseTab, this);
	//AddCmdIDHandler(IDM_CLOSEALLBUTTHISTAB, &CFilerTabGridView::OnCommandCloseAllButThisTab, this);
	//AddCmdIDHandler(IDM_ADDTOFAVORITE, &CFilerTabGridView::OnCommandAddToFavorite, this);
	//AddCmdIDHandler(IDM_OPENSAMEASOTHER, &CFilerTabGridView::OnCommandOpenSameAsOther, this);

}

CFilerTabGridView::~CFilerTabGridView() = default;

void CFilerTabGridView::OnCreate(const CreateEvent& e)
{
	//TextView Closure
	GetTextViewPtr = [spTextView = std::make_shared<CTextEditor>(GetWndPtr(), m_spTextEditorProp, nullptr, nullptr), isInitialized = false, this]()mutable->std::shared_ptr<CTextEditor>{

		if (!isInitialized && GetWndPtr()->IsWindow()) {
			isInitialized = true;
			//Path Changed
			spTextView->GetObsPath().Changed.connect([&](const NotifyChangedEventArgs<std::wstring>& e) {
				auto pData = std::static_pointer_cast<TextTabData>(m_itemsSource[m_selectedIndex.get()]);
				pData->Path = spTextView->GetObsPath().get();
				});

			//IsSave Changed
			spTextView->GetObsIsSaved().Changed.connect([&](const NotifyChangedEventArgs<bool>& e) {
				auto pData = std::static_pointer_cast<TextTabData>(m_itemsSource[m_selectedIndex.get()]);
				pData->IsSaved = spTextView->GetObsIsSaved().get();
				});
			//Create
			spTextView->OnCreate(CreateEvent(GetWndPtr(), NULL, NULL, nullptr));
		}
		return spTextView;
	};

	CTabControl::OnCreate(e);

	//ItemsSource
	if (m_itemsSource.empty()) {
		m_itemsSource.notify_push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
		m_itemsSource.notify_push_back(std::make_shared<ToDoTabData>(L""));
		m_itemsSource.notify_push_back(std::make_shared<TextTabData>(L""));
		m_selectedIndex.notify_set(0);
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
	auto headerRects = GetHeaderRects();
	auto iter = std::find_if(headerRects.begin(), headerRects.end(),
		[&](const d2dw::CRectF& rc)->bool { return rc.PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

	if (iter != headerRects.end()) {
		CMenu menu;
		menu.Attach(::GetSubMenu(::LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CONTEXTMENU_TAB)), 0));
		GetWndPtr()->SetForegroundWindow();
		menu.TrackPopupMenu(0, e.PointInScreen.x, e.PointInScreen.y, GetWndPtr()->m_hWnd);
	} else {
		m_spCurControl->OnContextMenu(e);
	}
}

void CFilerTabGridView::OnCommandNewFilerTab(const CommandEvent& e)
{
	m_itemsSource.notify_push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
	m_selectedIndex.notify_set(m_selectedIndex.get() - 1);
}

void CFilerTabGridView::OnCommandNewToDoTab(const CommandEvent& e)
{
	m_itemsSource.notify_push_back(std::make_shared<ToDoTabData>(L""));
	m_selectedIndex.notify_set(m_selectedIndex.get() - 1);
}

void CFilerTabGridView::OnCommandNewTextTab(const CommandEvent& e)
{
	m_itemsSource.notify_push_back(std::make_shared<TextTabData>(L""));
	m_selectedIndex.notify_set(m_selectedIndex.get() - 1);
}


void CFilerTabGridView::OnCommandCloneTab(const CommandEvent& e)
{
	m_itemsSource.notify_push_back(m_itemsSource[m_selectedIndex.get()]);
	m_selectedIndex.notify_set(m_selectedIndex - 1);
}

void CFilerTabGridView::OnCommandCloseTab(const CommandEvent& e)
{
	if(m_selectedIndex.get()>1){
		m_itemsSource.notify_erase(m_itemsSource.begin() + m_selectedIndex.get());
		m_selectedIndex.forth_notify_set((std::min)(m_selectedIndex.get() - 1, m_selectedIndex.get()));
	}
}

void CFilerTabGridView::OnCommandCloseAllButThisTab(const CommandEvent& e)
{
	if (m_selectedIndex.get()>1) {
		//Erase larger tab
		m_itemsSource.notify_erase(m_itemsSource.begin() + (m_selectedIndex.get() + 1), m_itemsSource.end());
		//Erase smaller tab
		m_itemsSource.notify_erase(m_itemsSource.begin(), m_itemsSource.begin() + m_selectedIndex.get());

		m_selectedIndex.forth_notify_set(0);
	}
}

void CFilerTabGridView::OnCommandAddToFavorite(const CommandEvent& e)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if(auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())){
		p->GetFavoritesPropPtr()->GetFavorites().notify_push_back(std::make_shared<CFavorite>(std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex])->Path, L""));
		//TODODO
		//p->GetLeftFavoritesView()->SubmitUpdate();
		//p->GetRightFavoritesView()->SubmitUpdate();
	}
}

void CFilerTabGridView::OnCommandOpenSameAsOther(const CommandEvent& e)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if (auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())) {
			
		std::shared_ptr<CFilerTabGridView> otherView = (this == p->GetLeftWnd().get())? p->GetRightWnd(): p->GetLeftWnd();
		m_itemsSource.notify_replace(m_itemsSource.begin() + m_selectedIndex.get(), otherView->GetItemsSource()[otherView->GetSelectedIndex()]);
	}
}




