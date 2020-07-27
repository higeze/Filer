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


CRect CFilerTabGridView::GetTabRect()
{
	int nPaddingX = GetSystemMetrics(SM_CXDLGFRAME);
	int nPaddingY = GetSystemMetrics(SM_CYDLGFRAME);

	CRect rcTabClient = GetClientRect();
	AdjustRect(FALSE, rcTabClient);
	rcTabClient.DeflateRect(nPaddingX, nPaddingY);
	return rcTabClient;
}


CFilerTabGridView::CFilerTabGridView(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:m_spFilerGridViewProp(spFilerGridViewProp)
{
	//FilerGridView Closure
	GetFilerGridViewPtr = [spFilerView = std::make_shared<CFilerGridView>(spFilerGridViewProp), this]()->std::shared_ptr<CFilerGridView> {

		if (!spFilerView->IsWindow() && m_hWnd) {
			DWORD dwStyle = spFilerView->CreateWindowExArgument().dwStyle();
			spFilerView->CreateWindowExArgument().dwStyle(dwStyle | WS_CHILD | WS_VISIBLE);
			spFilerView->Create(m_hWnd, GetTabRect());

			//Capture KeyDown Msg in FilerGridView
			spFilerView->AddMsgHandler(WM_KEYDOWN, &CFilerTabGridView::OnKeyDown, this);

			//Folder Changed
			spFilerView->FolderChanged = [&](std::shared_ptr<CShellFolder>& pFolder) {
				auto pData = std::static_pointer_cast<FilerTabData>(m_itemsSource[GetCurSel()]);
				pData->FolderPtr = pFolder;
				pData->Path = pFolder->GetPath();

				SetItemText(GetCurSel(), pData->GetItemText().c_str());

				spFilerView->PostUpdate(Updates::Rect);
				spFilerView->SetUpdateRect(spFilerView->GetDirectPtr()->Pixels2Dips(GetTabRect()));
			};
		}

		return spFilerView;
	};

	//ToDoGridView Closure
	GetToDoGridViewPtr = [spToDoView = std::make_shared<CToDoGridView>(std::static_pointer_cast<GridViewProperty>(m_spFilerGridViewProp)), this]()->std::shared_ptr<CToDoGridView>{

		if (!spToDoView->IsWindow() && m_hWnd) {
			spToDoView->RegisterClassExArgument()
				.lpszClassName(L"CBindWnd")
				.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
				.hCursor(::LoadCursor(NULL, IDC_ARROW))
				.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

			spToDoView->CreateWindowExArgument()
				.lpszClassName(_T("CBindWnd"))
				.lpszWindowName(L"TODO")
				.dwStyle(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
				.dwExStyle(WS_EX_ACCEPTFILES)
				.hMenu(NULL);

			spToDoView->AddMsgHandler(WM_KEYDOWN, &CFilerTabGridView::OnKeyDown, this);

			spToDoView->SetIsDeleteOnFinalMessage(false);

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
				auto pData = std::static_pointer_cast<ToDoTabData>(m_itemsSource[GetCurSel()]);
				pData->Path = spToDoView->GetObsPath().get();

				SetItemText(GetCurSel(), pData->GetItemText().c_str());

				spToDoView->PostUpdate(Updates::Rect);
				spToDoView->SetUpdateRect(spToDoView->GetDirectPtr()->Pixels2Dips(GetTabRect()));
			});

			spToDoView->Create(m_hWnd, GetTabRect());
		}
		return spToDoView;
	};

	//CFilerTabGridView
	CreateWindowExArgument()
		.dwStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | TCS_HOTTRACK | TCS_FLATBUTTONS | TCS_MULTILINE);

	//ItemsSource
	m_itemsSource.VectorChanged = [this](const NotifyVectorChangedEventArgs<std::shared_ptr<TabData>>& e)->void {
		switch (e.Action) {
			case NotifyVectorChangedAction::Add:
			{
				for (auto& pItem : e.NewItems) {
					InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, pItem->GetItemText().c_str(), NULL, NULL);
				}
			}
			break;
			case NotifyVectorChangedAction::Move:
				break;
			case NotifyVectorChangedAction::Remove:
			{
				for (int i = e.OldStartingIndex + e.OldItems.size() - 1; i >= e.OldStartingIndex; i--) {
					DeleteItem(i);
				}
			}
			break;

			case NotifyVectorChangedAction::Replace:
			{
				auto pData = m_itemsSource[m_selectedIndex];
				SetItemText(e.NewStartingIndex, pData->GetItemText().c_str());
				auto iter = m_itemsTemplate.find(typeid(*pData).name());
				if (iter != m_itemsTemplate.end()) {
					auto newView = iter->second(pData);
					if (newView && m_curView != newView) {
						if (m_curView) {
							m_curView->ShowWindow(SW_HIDE);
						}
						m_curView = newView;
						m_curView->ShowWindow(SW_SHOW);
						m_curView->MoveWindow(GetTabRect(), FALSE);
					} else {
						//Do nothing
					}
				}
			}
			break;
			case NotifyVectorChangedAction::Reset:
			{
				//Not implemented
			}
			break;
			default:
				break;
		}

	};


	//ItemsTemplate
	m_itemsTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CWnd> {
		auto pData = std::static_pointer_cast<FilerTabData>(pTabData);
		auto spView = GetFilerGridViewPtr();
		spView->OpenFolder(pData->FolderPtr);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CWnd> {
		auto pData = std::static_pointer_cast<ToDoTabData>(pTabData);
		auto spView = GetToDoGridViewPtr();
		spView->Open(pData->Path);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CWnd> {
		auto pData = std::static_pointer_cast<TextTabData>(pTabData);
		auto spView = GetTextViewPtr();
		spView->Open(pData->Path);

		return spView;
	});

	//SelectedIndex
	m_selectedIndex.Changed.connect(
		[this](const NotifyChangedEventArgs<int>& e)->void {
			SetCurSel(std::clamp(m_selectedIndex.get(), 0, GetItemCount() - 1));
			auto pData = m_itemsSource[m_selectedIndex];
			auto iter = m_itemsTemplate.find(typeid(*pData).name());
			if (iter != m_itemsTemplate.end()) {
				auto newView = iter->second(pData);
				if (newView && m_curView != newView) {
					if (m_curView) {
						m_curView->ShowWindow(SW_HIDE);
					}
					m_curView = newView;
					m_curView->ShowWindow(SW_SHOW);
					m_curView->MoveWindow(GetTabRect(), FALSE);
				} else {
					//Do nothing
				}
			}
		});


	AddMsgHandler(WM_CREATE, &CFilerTabGridView::OnCreate, this);
	AddMsgHandler(WM_SIZE, &CFilerTabGridView::OnSize, this);
	AddMsgHandler(WM_CLOSE, &CFilerTabGridView::OnClose, this);
	//AddMsgHandler(WM_DESTROY, &CFilerTabGridView::OnDestroy, this);
	AddMsgHandler(WM_KEYDOWN,&CFilerTabGridView::OnKeyDown,this);

	AddCmdIDHandler(IDM_NEWFILERTAB, &CFilerTabGridView::OnCommandNewFilerTab, this);
	AddCmdIDHandler(IDM_NEWTODOTAB, &CFilerTabGridView::OnCommandNewToDoTab, this);
	AddCmdIDHandler(IDM_NEWTEXTTAB, &CFilerTabGridView::OnCommandNewTextTab, this);

	AddCmdIDHandler(IDM_CLONETAB, &CFilerTabGridView::OnCommandCloneTab, this);
	AddCmdIDHandler(IDM_CLOSETAB, &CFilerTabGridView::OnCommandCloseTab, this);
	AddCmdIDHandler(IDM_CLOSEALLBUTTHISTAB, &CFilerTabGridView::OnCommandCloseAllButThisTab, this);
	AddCmdIDHandler(IDM_ADDTOFAVORITE, &CFilerTabGridView::OnCommandAddToFavorite, this);
	AddCmdIDHandler(IDM_OPENSAMEASOTHER, &CFilerTabGridView::OnCommandOpenSameAsOther, this);
}

LRESULT CFilerTabGridView::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//TextView Closure
	GetTextViewPtr = [spTextView = std::make_shared<CTextboxWnd>(std::make_shared<TextboxProperty>()), this]()->std::shared_ptr<CTextboxWnd>{

		if (!spTextView->IsWindow() && m_hWnd) {
			spTextView->RegisterClassExArgument()
				.lpszClassName(L"CTextView")
				.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
				.hCursor(::LoadCursor(NULL, IDC_ARROW))
				.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

			spTextView->CreateWindowExArgument()
				.lpszClassName(_T("CTextView"))
				.lpszWindowName(L"Text")
				.dwStyle(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
				.dwExStyle(WS_EX_ACCEPTFILES)
				.hMenu(NULL);

			spTextView->AddMsgHandler(WM_KEYDOWN, &CFilerTabGridView::OnKeyDown, this);

			spTextView->SetIsDeleteOnFinalMessage(false);


			//Path Changed
			spTextView->GetObsPath().Changed.connect([&](const NotifyChangedEventArgs<std::wstring>& e) {
				auto pData = std::static_pointer_cast<ToDoTabData>(m_itemsSource[GetCurSel()]);
				pData->Path = spTextView->GetObsPath().get();

				SetItemText(GetCurSel(), pData->GetItemText().c_str());

				spTextView->MoveWindow(GetTabRect(), FALSE);
													 });

			spTextView->Create(m_hWnd, GetTabRect());
		}
		return spTextView;
	};



	//ItemsSource
	if (m_itemsSource.empty()) {
		m_itemsSource.notify_push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
		m_itemsSource.notify_push_back(std::make_shared<ToDoTabData>(L""));
		m_itemsSource.notify_push_back(std::make_shared<TextTabData>(L""));
	} else {
		for (auto& pItem : m_itemsSource) {
			InsertItem(GetItemCount(), TCIF_PARAM | TCIF_TEXT, pItem->GetItemText().c_str(), NULL, NULL);
		}
	}

	//Font
	m_font = m_spFilerGridViewProp->HeaderPropPtr->Format->Font.GetGDIFont();
	SetFont(m_font, TRUE);

	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), NM_CLICK, &CFilerTabGridView::OnNotifyTabLClick, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), NM_RCLICK, &CFilerTabGridView::OnNotifyTabRClick, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_SELCHANGING, &CFilerTabGridView::OnNotifyTabSelChanging, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_SELCHANGE, &CFilerTabGridView::OnNotifyTabSelChange, this);
	m_pParentWnd->AddNtfyHandler((UINT_PTR)m_cwa.hMenu(), TCN_KEYDOWN, [this](int id, LPNMHDR pnmh, BOOL& bHandled)->LRESULT {
		this->OnKeyDown(WM_KEYDOWN, (WPARAM)((NMTCKEYDOWN*)pnmh)->wVKey, NULL, bHandled);
		return 0;
	});

	SetCurSel(std::clamp(m_selectedIndex.get(), 0, GetItemCount() -1));

	//Show Current view
	auto pData = m_itemsSource[GetCurSel()];
	auto iter = m_itemsTemplate.find(typeid(*pData).name());
	if (iter != m_itemsTemplate.end()) {
		m_curView = iter->second(pData);
	}

	return 0;
}

LRESULT CFilerTabGridView::OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	GetFilerGridViewPtr()->DestroyWindow();
	GetToDoGridViewPtr()->DestroyWindow();
	GetTextViewPtr()->DestroyWindow();
	DestroyWindow();
	return 0;
}

LRESULT CFilerTabGridView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_curView->IsWindow()) {
		m_curView->MoveWindow(GetTabRect(), FALSE);
	}
	return 0;
}

LRESULT CFilerTabGridView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (wParam)
	{
	case 'T':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			SendMessage(WM_COMMAND, IDM_NEWFILERTAB, NULL);
		}
		break;
	case 'W':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			SendMessage(WM_COMMAND, IDM_CLOSETAB, NULL);
		}
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabSelChanging(int, LPNMHDR, BOOL& bHandled)
{
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabSelChange(int, LPNMHDR, BOOL& bHandled)
{
	m_selectedIndex.notify_set(GetCurSel());
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabLClick(int id, LPNMHDR, BOOL& bHandled)
{
	::SetFocus(m_curView->m_hWnd);
	return 0;
}

LRESULT CFilerTabGridView::OnNotifyTabRClick(int id, LPNMHDR, BOOL& bHandled)
{
	CPoint ptScreen;
	::GetCursorPos(ptScreen);
	CPoint ptClient(ptScreen);
	ScreenToClient(ptClient);
	TC_HITTESTINFO tchi = { 0 };
	tchi.pt = ptClient;
	tchi.flags = TCHT_ONITEMLABEL;
	int index = HitTest(&tchi);
	SetContextMenuTabIndex(index);

	CMenu menu;
	menu.Attach(::GetSubMenu(::LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CONTEXTMENU_TAB)), 0));
	::SetForegroundWindow(m_hWnd);
	menu.TrackPopupMenu(0, ptScreen.x, ptScreen.y, m_hWnd);

	return 0;
}

LRESULT CFilerTabGridView::OnCommandNewFilerTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_itemsSource.notify_push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
	m_selectedIndex.notify_set(GetItemCount() - 1);
	return 0;
}

LRESULT CFilerTabGridView::OnCommandNewToDoTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_itemsSource.notify_push_back(std::make_shared<ToDoTabData>(L""));
	m_selectedIndex.notify_set(GetItemCount() - 1);
	return 0;
}

LRESULT CFilerTabGridView::OnCommandNewTextTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_itemsSource.notify_push_back(std::make_shared<TextTabData>(L""));
	m_selectedIndex.notify_set(GetItemCount() - 1);
	return 0;
}


LRESULT CFilerTabGridView::OnCommandCloneTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_itemsSource.notify_push_back(m_itemsSource[m_contextMenuTabIndex]);
	m_selectedIndex.notify_set(GetItemCount() - 1);
	return 0;
}

LRESULT CFilerTabGridView::OnCommandCloseTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(GetItemCount()>1){
		m_itemsSource.notify_erase(m_itemsSource.begin() + m_contextMenuTabIndex);
		m_selectedIndex.forth_notify_set((std::min)(GetItemCount() - 1, m_contextMenuTabIndex));
	}
	return 0;
}

LRESULT CFilerTabGridView::OnCommandCloseAllButThisTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (GetItemCount()>1) {
		//Erase larger tab
		m_itemsSource.notify_erase(m_itemsSource.begin() + (m_contextMenuTabIndex + 1), m_itemsSource.end());
		//Erase smaller tab
		m_itemsSource.notify_erase(m_itemsSource.begin(), m_itemsSource.begin() + m_contextMenuTabIndex);

		m_selectedIndex.forth_notify_set(0);
	}
	return 0;
}

LRESULT CFilerTabGridView::OnCommandAddToFavorite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if(auto p = dynamic_cast<CFilerWnd*>(m_pParentWnd)){
		p->GetFavoritesPropPtr()->GetFavorites().notify_push_back(std::make_shared<CFavorite>(std::static_pointer_cast<FilerTabData>(m_itemsSource[m_contextMenuTabIndex])->Path, L""));
		p->GetLeftFavoritesView()->SubmitUpdate();
		p->GetRightFavoritesView()->SubmitUpdate();
	}
	return 0;
}

LRESULT CFilerTabGridView::OnCommandOpenSameAsOther(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if (auto p = dynamic_cast<CFilerWnd*>(m_pParentWnd)) {
			
		std::shared_ptr<CFilerTabGridView> otherView = (this == p->GetLeftView().get())? p->GetRightView(): p->GetLeftView();
		m_itemsSource.notify_replace(m_itemsSource.begin() + m_selectedIndex.get(), otherView->GetItemsSource()[otherView->GetSelectedIndex()]);
	}
	return 0;
}




