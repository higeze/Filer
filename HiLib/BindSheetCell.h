#pragma once
#include "SheetCell.h"
#include "observable.h"

template<typename T>
class CBindSheetCell :public CSheetCell
{
private:
	observable_vector<T> m_itemsSource;
public:
	CBindSheetCell(
		CSheet* pSheet = nullptr,
		CRow* pRow = nullptr,
		CColumn* pColumn = nullptr,
		std::shared_ptr<CellProperty> spProperty = nullptr,
		std::shared_ptr<HeaderProperty> spHeaderProperty = nullptr,
		std::shared_ptr<CellProperty> spFilterProperty = nullptr,
		std::shared_ptr<CellProperty> spCellProperty = nullptr)
		:CSheetCell(pSheet, pRow, pColumn, spProperty, spHeaderProperty, spFilterProperty, spCellProperty)
	{
		m_itemsSource.VectorChanged.connect(
			[this](const NotifyVectorChangedEventArgs<T>& e)->void {
				switch (e.Action) {
				case NotifyVectorChangedAction::Add:
					PushRow(std::make_shared<CBindRow<T>>(this));
					break;
				case NotifyVectorChangedAction::Remove:
					EraseRow(m_allRows.back());
					break;
				case NotifyVectorChangedAction::Reset:
					//TODOTODO
				default:
					break;
				}

			}
		);
	}

	virtual bool HasSheetCell()override { return false; }
	virtual bool IsVirtualPage()override { return true; }

	observable_vector<T>& GetItemsSource() { return m_itemsSource; }

	/******************/
	/* Window Message */
	/******************/

	/****************/
	/* StateMachine */
	/****************/
	void Normal_ContextMenu(const ContextMenuEvent& e) override
	{
		//CreateMenu
		CMenu menu(::CreatePopupMenu());
		//Add Row
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_TYPE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row");
		mii.dwTypeData = L"Add Row";
		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row");
		mii.dwTypeData = L"Remove Row";
		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

		CPoint ptClient(e.Point);
		CPoint ptScreen(e.Point);
		ClientToScreen(ptScreen);
		::SetForegroundWindow(m_hWnd);
		int idCmd = menu.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
			ptScreen.x,
			ptScreen.y,
			m_hWnd);

		if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row")) {
			m_itemsSource.notify_push_back(T());
		} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row")) {
			auto a = Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>();
			m_itemsSource.notify_erase(m_itemsSource.cbegin() + Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>() - m_frozenRowCount);
		}

	}

};
