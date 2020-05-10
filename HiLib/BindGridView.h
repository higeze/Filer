#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "observable.h"
#include "MyString.h"
#include "MyXmlSerializer.h"
#include "IBindSheet.h"
#include "Cursorer.h"


template<typename... TItems>
class CBindGridView :public CGridView
{
protected:
	std::shared_ptr<observable_vector<std::tuple<TItems...>>> m_spItemsSource;

public:
	template<typename... TArgs> 
	CBindGridView(std::shared_ptr<GridViewProperty>& spGridViewProp,
				  std::shared_ptr<observable_vector<std::tuple<TItems...>>> spItemsSource = nullptr,
				  TArgs... args)
		:CGridView(spGridViewProp), m_spItemsSource(spItemsSource)
	{
		//ItemsSource
		if (!m_spItemsSource) {
			m_spItemsSource = std::make_shared<observable_vector<std::tuple<TItems...>>>();
		}
		//VectorChanged
		auto& itemsSource = GetItemsSource();
		auto funVectorChanged =
			[this](const NotifyVectorChangedEventArgs<std::tuple<TItems...>>& e)->void {
			switch (e.Action) {
				case NotifyVectorChangedAction::Add:
					PushRow(std::make_shared<CBindRow<TItems...>>(this));
					break;
				case NotifyVectorChangedAction::Insert:
					InsertRow(e.NewStartingIndex, std::make_shared<CBindRow<TItems...>>(this));
					break;
				case NotifyVectorChangedAction::Remove:
				{
					auto spRow = m_allRows[e.OldStartingIndex + m_frozenRowCount];
					for (const auto& colPtr : m_allCols) {
						if (auto pMapCol = std::dynamic_pointer_cast<CMapColumn>(colPtr)) {
							pMapCol->Erase(const_cast<CRow*>(spRow.get()));
						}
					}
					EraseRow(spRow);
					break;
				}
				case NotifyVectorChangedAction::Reset:
					m_allRows.idx_erase(m_allRows.begin() + m_frozenRowCount, m_allRows.end());
					break;
				default:
					break;
			}
		};

		if (itemsSource.VectorChanged) {
			itemsSource.VectorChanged =
				[existingFun = itemsSource.VectorChanged, newFun = funVectorChanged](const NotifyVectorChangedEventArgs<std::tuple<TItems...>>& e)->void {
				existingFun(e);
				newFun(e);
			};
		} else {
			itemsSource.VectorChanged = funVectorChanged;
		}

		//TArg...
		m_pHeaderColumn = ::get(arg<"hdrcol"_s>(), args..., default_(nullptr));
		if (m_pHeaderColumn) {
			m_pHeaderColumn->SetSheetPtr(this);
			PushColumn(m_pHeaderColumn);
		}

		m_pNameColumn = ::get(arg<"namecol"_s>(), args..., default_(nullptr));
		if (m_pNameColumn) {
			m_pNameColumn->SetSheetPtr(this);
			PushColumn(m_pNameColumn);
		}

		m_pHeaderRow = ::get(arg<"hdrrow"_s>(), args..., default_(nullptr));
		if (m_pHeaderRow){
			m_pHeaderRow->SetSheetPtr(this);
			PushRow(m_pHeaderRow);
		}

		m_pNameHeaderRow = ::get(arg<"namerow"_s>(), args..., default_(nullptr));
		if (m_pNameHeaderRow) {
			m_pNameHeaderRow->SetSheetPtr(this);
			PushRow(m_pNameHeaderRow);
		}

		m_pFilterRow = ::get(arg<"fltrow"_s>(), args..., default_(nullptr));
		if (m_pFilterRow) {
			m_pFilterRow->SetSheetPtr(this);
			PushRow(m_pFilterRow);
		}

		m_frozenColumnCount = ::get(arg<"frzcolcnt"_s>(), args..., default_(0));
		m_frozenRowCount = ::get(arg<"frzrowcnt"_s>(), args..., default_(0));

		std::vector<std::shared_ptr<CColumn>> columns;
		columns = ::get(arg<"columns"_s>(), args..., default_(columns));
		for (auto& spCol : columns) {
			spCol->SetSheetPtr(this);
			PushColumn(spCol);
		}

		//PushNewRow
		for (auto& tup : itemsSource) {
			PushRow(std::make_shared<CBindRow<TItems...>>(this));
		}
	}

	observable_vector<std::tuple<TItems...>>& GetItemsSource() { return *m_spItemsSource; }
	//std::vector<std::tuple<TItems...>>& GetSelectedItems() { return m_funSelItems(); }
};

//
//template<typename TItem>
//class CBindGridViewTest :public CGridView, public IBindSheet<TItem>
//{
//private:
//	observable_vector<TItem> m_itemsSource;
//public:
//	CBindGridViewTest(std::shared_ptr<GridViewProperty>& spGridViewProp)
//		:CGridView(spGridViewProp)
//	{
//		m_spCursorer = std::make_shared<CExcelLikeCursorer>();
//
//		m_itemsSource.VectorChanged =
//			[this](const NotifyVectorChangedEventArgs<TItem>& e)->void {
//				switch (e.Action) {
//				case NotifyVectorChangedAction::Add:
//					PushRow(std::make_shared<CBindRowTest<TItem>>(this));
//					break;
//				case NotifyVectorChangedAction::Remove:
//					EraseRow(m_allRows.back());
//					break;
//				case NotifyVectorChangedAction::Reset:
//					//TODOTODO
//				default:
//					break;
//				}
//
//			};
//	}
//
//	virtual bool HasSheetCell()override { return true; }
//	virtual bool IsVirtualPage()override { return true; }
//
//	observable_vector<TItem>& GetItemsSource() override { return m_itemsSource; }
//
//	void RowMoved(CMovedEventArgs<RowTag>& e) override
//	{
//		auto& itemsSource = GetItemsSource();
//		auto fromIter = itemsSource.cbegin() + (e.m_from - GetFrozenCount<RowTag>());
//		auto temp = *fromIter;
//		itemsSource.erase(fromIter);
//		auto toIter = itemsSource.cbegin() + (e.m_to - GetFrozenCount<RowTag>());
//		itemsSource.insert(toIter, temp);
//	}
//
//	/******************/
//	/* Window Message */
//	/******************/
//
//	/****************/
//	/* StateMachine */
//	/****************/
//
//	void Normal_ContextMenu(const ContextMenuEvent& e) override
//	{
//		auto spCell = Cell(m_pDirect->Pixels2Dips(e.Point));
//		if (spCell) {
//			spCell->OnContextMenu(e);
//			if (e.Handled) { return; }
//		}
//
//		//CreateMenu
//		CMenu menu(::CreatePopupMenu());
//		//Add Row
//		MENUITEMINFO mii = { 0 };
//		mii.cbSize = sizeof(MENUITEMINFO);
//		mii.fMask = MIIM_TYPE | MIIM_ID;
//		mii.fType = MFT_STRING;
//		mii.fState = MFS_ENABLED;
//		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row");
//		mii.dwTypeData = L"Add Row";
//		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
//
//		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row");
//		mii.dwTypeData = L"Remove Row";
//		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
//
//		CPoint ptClient(e.Point);
//		CPoint ptScreen(e.Point);
//		ClientToScreen(ptScreen);
//		::SetForegroundWindow(m_hWnd);
//		int idCmd = menu.TrackPopupMenu(
//			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
//			ptScreen.x,
//			ptScreen.y,
//			m_hWnd);
//
//		if(idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row")){
//			m_itemsSource.notify_push_back(TItem());
//		} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row")) {
//			auto a = Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>();
//			m_itemsSource.notify_erase(m_itemsSource.cbegin() + (Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>() - m_frozenRowCount));
//		}
//		e.Handled = TRUE;
//	}
//
//	void Normal_KeyDown(const KeyDownEvent& e) override
//	{
//		switch (e.Char) {
//		case 'O':
//			if (::GetAsyncKeyState(VK_CONTROL)) {
//
//				std::wstring path;
//				OPENFILENAME ofn = { 0 };
//				ofn.lStructSize = sizeof(OPENFILENAME);
//				ofn.hwndOwner = m_hWnd;
//				ofn.lpstrFilter = L"XML file(*.xml)\0*.xml\0\0";
//				ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
//				ofn.nMaxFile = MAX_PATH;
//				ofn.lpstrTitle = L"Open";
//				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
//				ofn.lpstrDefExt = L"xml";
//
//				if (!GetOpenFileName(&ofn)) {
//					DWORD errCode = CommDlgExtendedError();
//					if (errCode) {
//						//wsprintf(szErrMsg, L"Error code : %d", errCode);
//						//MessageBox(NULL, szErrMsg, L"GetOpenFileName", MB_OK);
//					}
//				} else {
//					::ReleaseBuffer(path);
//					//Deserialize
//					std::vector<TItem> itemsSource;
//					try {
//						//Serialize
//						CXMLSerializer<std::vector<TItem>> serializer;
//						serializer.Deserialize(path.c_str(), L"Task", itemsSource);
//						//m_itemsSource.notify_clear();//TODOTODO
//						while (!m_itemsSource.empty()) {
//							m_itemsSource.notify_erase(m_itemsSource.cbegin());
//						}
//						for (const auto& item : itemsSource) {
//							m_itemsSource.notify_push_back(item);
//						}
//					}
//					catch (/*_com_error &e*/...) {
//					}
//				}
//			}
//			break;
//		case 'S':
//			if (::GetAsyncKeyState(VK_CONTROL)) {
//
//				std::wstring path;
//				OPENFILENAME ofn = { 0 };
//				ofn.lStructSize = sizeof(OPENFILENAME);
//				ofn.hwndOwner = m_hWnd;
//				ofn.lpstrFilter = L"XML file(*.xml)\0*.xml\0\0";
//				ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
//				ofn.nMaxFile = MAX_PATH;
//				ofn.lpstrTitle = L"Save as";
//				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
//				ofn.lpstrDefExt = L"xml";
//
//				if (!GetSaveFileName(&ofn)) {
//					DWORD errCode = CommDlgExtendedError();
//					if (errCode) {
//						//wsprintf(szErrMsg, L"Error code : %d", errCode);
//						//MessageBox(NULL, szErrMsg, L"GetOpenFileName", MB_OK);
//					}
//				} else {
//					::ReleaseBuffer(path);
//					//Serialize
//					try {
//						std::vector<TItem> itemsSource;
//						std::copy(m_itemsSource.cbegin(), m_itemsSource.cend(), std::back_inserter(itemsSource));
//						CXMLSerializer<std::vector<TItem>> serializer;
//						serializer.Serialize(path.c_str(), L"Task", itemsSource);
//					}
//					catch (/*_com_error &e*/...) {
//					}
//				}
//			}
//			break;
//		//case VK_F2:
//		//{
//		//	//TODOTODO
//		//	//if (m_spCursorer->GetFocusedCell()) {
//		//	//	std::static_pointer_cast<CFileIconNameCell>(CSheet::Cell(m_spCursorer->GetFocusedCell()->GetRowPtr(), m_pNameColumn.get()))->OnEdit(EventArgs(this));
//		//	//}
//		//}
//		//break;
//
//		default:
//			CGridView::Normal_KeyDown(e);
//		}
//	}
//
//};