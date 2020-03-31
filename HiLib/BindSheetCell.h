#pragma once
#include "SheetCell.h"
#include "BindSheetCellColumn.h"
#include "IBindSheet.h"
#include "observable.h"

template<typename TItem, typename TValueItem>
class CBindSheetCell :public CSheetCell, public IBindSheet<TValueItem>
{
public:
	CBindSheetCell(
		CSheet* pSheet,
		CRow* pRow,
		CColumn* pColumn,
		std::shared_ptr<SheetProperty> spSheetProperty,
		std::shared_ptr<CellProperty> spCellProperty,
		std::function<void(CBindSheetCell<TItem, TValueItem>*)> initializer)
		:CSheetCell(pSheet, pRow, pColumn, spSheetProperty, spCellProperty)
	{
		initializer(this);

		auto& itemsSource = GetItemsSource();

		for (auto& item : itemsSource) {
			PushRow(std::make_shared<CBindRow<TValueItem>>(this));
		}

		GetItemsSource().VectorChanged.connect(
			[this](const NotifyVectorChangedEventArgs<TValueItem>& e)->void {
				switch (e.Action) {
				case NotifyVectorChangedAction::Add:
					PushRow(std::make_shared<CBindRow<TValueItem>>(this));
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

	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

	observable_vector<TValueItem>& GetItemsSource() override
	{
		auto pBindRow = static_cast<CBindRow<TItem>*>(m_pRow);
		auto pBindColumn = static_cast<CBindSheetCellColumn<TItem, TValueItem>*>(m_pColumn);
		return pBindColumn->GetItemser()(pBindRow->GetItem());
	}

	/******************/
	/* Window Message */
	/******************/

	/****************/
	/* StateMachine */
	/****************/
	void Normal_ContextMenu(const ContextMenuEvent& e) override
	{
		auto spCell = Cell(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.Point));
		if (spCell) {
			spCell->OnContextMenu(e);
			if (e.Handled) { return; }
		}

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
		e.WndPtr->ClientToScreen(ptScreen);
		::SetForegroundWindow(e.WndPtr->m_hWnd);
		int idCmd = menu.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
			ptScreen.x,
			ptScreen.y,
			e.WndPtr->m_hWnd);

		if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row")) {
			GetItemsSource().notify_push_back(TValueItem());
		} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row")) {
			auto a = Cell(e.WndPtr->GetDirectPtr()->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>();
			GetItemsSource().notify_erase(GetItemsSource().cbegin() + Cell(e.WndPtr->GetDirectPtr()->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>() - m_frozenRowCount);
		}
		e.Handled = TRUE;

	}

};
