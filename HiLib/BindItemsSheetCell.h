#pragma once
#include "BindItemsSheetCellBase.h"
#include "BindSheetCellColumn.h"
#include "IBindSheet.h"
#include "ReactiveProperty.h"

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
			PushRow(std::make_shared<CBindRow<TValueItem>>(this));
		}

		items.SubscribeDetail(
			[this](const NotifyVectorChangedEventArgs<std::tuple<TValueItem>>& e)->void {
			switch (e.Action) {
				case NotifyVectorChangedAction::Add:
					PushRow(std::make_shared<CBindRow<TValueItem>>(this));
					break;
				case NotifyVectorChangedAction::Insert:
					InsertRow(e.NewStartingIndex, std::make_shared<CBindRow<TValueItem>>(this));
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
		});
	}

	ReactiveVectorProperty<std::tuple<TValueItem>>& GetItemsSourceImpl()
	{
		auto pBindRow = static_cast<CBindRow<TItem>*>(m_pRow);
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
