#pragma once
#include "SheetCell.h"
#include "IBindSheet.h"
#include "observable.h"
#include "Debug.h"
#include "named_arguments.h"

template<typename TValueItem>
class CBindItemsSheetCellBase :public CSheetCell, public IBindSheet<TValueItem>
{
private:
	std::function<observable_vector<TValueItem>& (CSheetCell*)> m_funItems = nullptr;
	observable_vector<TValueItem>* m_ptrItems = nullptr;
public:
	template<typename... Args>
	CBindItemsSheetCellBase(
		CSheet* pSheet,
		CRow* pRow,
		CColumn* pColumn,
		std::shared_ptr<SheetProperty> spSheetProperty,
		std::shared_ptr<CellProperty> spCellProperty,
		Args... args)
		:CSheetCell(pSheet, pRow, pColumn, spSheetProperty, spCellProperty)
	{
		auto rowHeaders = ::get(arg<"rowhdrs"_s>(), args..., default_(std::vector<std::shared_ptr<CRow>>()));
		auto columnHeaders = ::get(arg<"colhdrs"_s>(), args..., default_(std::vector<std::shared_ptr<CColumn>>()));
		m_funItems = ::get(arg<"funitems"_s>(), args..., default_(nullptr));
		m_ptrItems = ::get(arg<"ptritems"_s>(), args..., default_(nullptr));
	}

	virtual ~CBindItemsSheetCellBase() = default;

	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

	observable_vector<TValueItem>& GetItemsSource() 
	{ 
		if (m_funItems) {
			return m_funItems(this);
		} else if (m_ptrItems) {
			return *m_ptrItems;
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	void RowMoved(CMovedEventArgs<RowTag>& e) override
	{
		auto& itemsSource = GetItemsSource();
		auto fromIter = itemsSource.cbegin() + (e.m_from - GetFrozenCount<RowTag>());
		auto temp = *fromIter;
		itemsSource.erase(fromIter);
		auto toIter = itemsSource.cbegin() + (e.m_to - GetFrozenCount<RowTag>());
		itemsSource.insert(toIter, temp);
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
			auto& items = GetItemsSource();
			items.notify_push_back(TValueItem());
		} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row")) {
			auto a = Cell(e.WndPtr->GetDirectPtr()->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>();
			this->GetItemsSource().notify_erase(this->GetItemsSource().cbegin() + (Cell(e.WndPtr->GetDirectPtr()->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>() - m_frozenRowCount));
		}
		e.Handled = TRUE;

	}

};
