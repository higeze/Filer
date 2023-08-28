#pragma once
#include "BindItemsSheetCellBase.h"
#include "BindSheetCellColumn.h"

template<typename TItem, typename TValueItem>
class CBindItemsSheetCell :public CBindItemsSheetCellBase<TValueItem>
{
public:
	template<typename... Args>
	CBindItemsSheetCell(
		CSheet* pSheet,
		CRow* pRow,
		CColumn* pColumn,
		std::shared_ptr<SheetProperty> spSheetProperty,
		std::shared_ptr<CellProperty> spCellProperty,
		std::function<void(CBindItemsSheetCell<TItem, TValueItem>*)> initializer,
		Args... args)
		:CBindItemsSheetCellBase<TValueItem>(
			pSheet, pRow, pColumn, spSheetProperty, spCellProperty,
			arg<"funitems"_s>() = [](CSheetCell* p)->ReactiveVectorProperty<std::tuple<TValueItem>>&{ return static_cast<CBindItemsSheetCell<TItem, TValueItem>*>(p)->GetItemsSourceImpl();},
			args...)
	{
		initializer(this);

		auto& items = this->GetItemsSource();

		for (auto& item : items) {
			this->PushRow(std::make_shared<CBindRow<TValueItem>>(this, spCellProperty));
		}

		items.Subscribe(
			[this](const NotifyVectorChangedEventArgs<std::tuple<TValueItem>>& e)->void {
			switch (e.Action) {
				case NotifyVectorChangedAction::Add:
					this->PushRow(std::make_shared<CBindRow<TValueItem>>(this, spCellProperty));
					break;
				case NotifyVectorChangedAction::Insert:
					this->InsertRow(e.NewStartingIndex, std::make_shared<CBindRow<TValueItem>>(this, spCellProperty));
					break;
				case NotifyVectorChangedAction::Remove:
				{
					auto spRow = this->m_allRows[e.OldStartingIndex + this->m_frozenRowCount];
					this->m_allCells.get<row_tag>().erase(spRow.get());
					this->EraseRow(spRow);
					break;
					break;
				}
				case NotifyVectorChangedAction::Reset:
					this->m_allRows.idx_erase(this->m_allRows.begin() + this->m_frozenRowCount, this->m_allRows.end());
					break;
				default:
					break;
			}
		});
	}

	ReactiveVectorProperty<std::tuple<TValueItem>>& GetItemsSourceImpl()
	{
		auto pBindRow = static_cast<CBindRow<TItem>*>(this->m_pRow);
		auto pBindColumn = static_cast<const CBindSheetCellColumn<TItem, TValueItem>*>(this->m_pColumn);
		return pBindColumn->GetItemser()(pBindRow->GetTupleItems());
	}

	/******************/
	/* Window Message */
	/******************/

	/****************/
	/* StateMachine */
	/****************/
};
