#pragma once
#include "Column.h"
#include "TextCell.h"
#include "Debug.h"
#include "IBindSheet.h"
#include "Debug.h"

template<typename... TItems>
class CBindColumn :public CColumn
{
public:
	CBindColumn(CSheet* pSheet)
		:CColumn(pSheet){ }

	std::tuple<TItems...>& GetTupleItems()
	{
		if(auto pBindSheet = dynamic_cast<IBindSheet<TItems...>*>(this->m_pSheet)){
			auto& itemsSource = pBindSheet->GetItemsSource();
			auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<ColTag>();

			return itemsSource[index];

		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<typename TItem> TItem& GetItem()
	{
		return std::get<TItem>(GetTupleItems());
	}

	std::shared_ptr<CCell>& Cell(CRow* pRow )
	{
		if (pRow->HasCell()) {
			return pRow->Cell(this);
		} else {
			THROW_FILE_LINE_FUNC;
		}
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CTextCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
	}

};

