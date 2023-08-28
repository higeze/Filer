#pragma once
#include "BindItemsSheetCellBase.h"
#include "BindSheetCellColumn.h"

template<typename TItem, typename TValueItem>
class CBindItemsFilterSheetCell :public CBindItemsSheetCellBase<TValueItem>
{
public:
	template<typename... Args>
	CBindItemsFilterSheetCell(
		CSheet* pSheet,
		CRow* pRow,
		CColumn* pColumn,
		std::shared_ptr<SheetProperty> spSheetProperty,
		std::shared_ptr<CellProperty> spCellProperty,
		std::function<void(CBindItemsSheetCell<TItem, TValueItem>*)> initializer,
		Args... args)
		:CBindItemsSheetCellBase<TValueItem>(
			pSheet, pRow, pColumn, spSheetProperty, spCellProperty,
			arg<"funitems"_s>() = [](CSheetCell* p)->ReactiveVectorProperty<TValueItem>& { return static_cast<CBindItemsSheetCell<TItem, TValueItem>*>(p)->GetItemsSourceImpl(); },
			args...)
	{
		initializer(this);

		auto& items = this->GetItemsSource();

		for (auto& item : items) {
			PushRow(std::make_shared<CBindRowTest<TValueItem>>(this, GetCellProperty()));
		}

		items.VectorChanged =
			[this](const NotifyVectorChangedEventArgs<TValueItem>& e)->void {
			switch (e.Action) {
			case NotifyVectorChangedAction::Add:
				PushRow(std::make_shared<CBindRowTest<TValueItem>>(this, GetCellProperty()));
				break;
			case NotifyVectorChangedAction::Remove:
				EraseRow(m_allRows.back());
				break;
			case NotifyVectorChangedAction::Reset:
				//TODOTODO
			default:
				break;
			}

		};
	}

	ReactiveVectorProperty<TValueItem>& GetItemsSourceImpl()
	{
		auto pBindRow = static_cast<CBindRowTest<TItem>*>(m_pRow);
		auto pBindColumn = static_cast<CBindSheetCellColumn<TItem, TValueItem>*>(m_pColumn);
		return pBindColumn->GetItemser()(pBindRow->GetTupleItems());
	}

	/******************/
	/* Window Message */
	/******************/

	/****************/
	/* StateMachine */
	/****************/
};
