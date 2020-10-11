#pragma once
#include "BindItemsSheetCellBase.h"
#include "Row.h"
#include "PropertySerializer.h"
#include "SheetEventArgs.h"
#include "CellProperty.h"
#include "MyMPL.h"
#include <memory>

template<typename TValueItem>
class CBindPropertySheetCell
	:public CBindItemsSheetCellBase<TValueItem>//,std::enable_shared_from_this<CBindPropertySheetCell<TValueItem>>
{
public:
	virtual bool CanResizeRow()const override{return true;}
	virtual bool CanResizeColumn()const override{return false;}
	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

public:

	//Constructor
	template<typename... Args>
	CBindPropertySheetCell(
		CSheet* pSheet,
		CRow* pRow,
		CColumn* pColumn,
		std::shared_ptr<SheetProperty>& spSheetProperty,
		std::shared_ptr<CellProperty>& spCellProperty,
		ReactiveProperty<std::tuple<TValueItem>>& itemsSource,
		Args... args)
		:CBindItemsSheetCellBase<TValueItem>(
			pSheet,pRow,pColumn,spSheetProperty, spCellProperty,
			arg<"items"_s>() = itemsSource,
			args...)
	{
		SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(this));
		InsertRow(0, GetNameHeaderRowPtr());
		SetFrozenCount<RowTag>(1);

		auto spColValue = std::make_shared<CPropertyValueColumn>(this);
		PushColumns(
			std::make_shared<CRowIndexColumn>(this),
			spColValue);
		SetFrozenCount<ColTag>(1);

		CCellSerializer serializer(this);
		auto& items = GetItemsSource();
		for (auto& val : items) {
			auto spRow = std::make_shared<CBindRow<TValueItem>>(this);
			PushRow(spRow);
			serializer.SerializeValue(val, spRow.get(), spColValue.get());
		}

		items.VectorChanged = 
			[this, spColValue](const NotifyVectorChangedEventArgs<std::tuple<TValueItem>>& e)->void {
				switch (e.Action) {
				case NotifyVectorChangedAction::Add:
				{
					auto spRow = std::make_shared<CBindRow<TValueItem>>(this);
					PushRow(spRow);
					CCellSerializer serializer(this);
					auto val = CreateInstance<TValueItem>();
					serializer.SerializeValue(val, spRow.get(), spColValue.get());
					break;
				}
				case NotifyVectorChangedAction::Remove:
				{
					EraseRow(m_allRows.back());
					break;
				}
				case NotifyVectorChangedAction::Reset:
					//TODOTODO
				default:
					break;
				}
			};

	}
};
