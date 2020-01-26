#pragma once
#include "CheckBoxCell.h"
#include "BindCheckBoxColumn.h"

template<typename T>
class CBindCheckBoxCell :public CCheckBoxCell
{
public:
	CBindCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CCheckBoxCell(pSheet, pRow, pColumn, spProperty){}
	virtual ~CBindCheckBoxCell() = default;

	virtual bool GetCheck() const override
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		auto pBindColumn = static_cast<CBindCheckBoxColumn<T>*>(m_pColumn);
		auto pBindGrid = static_cast<CBindGridView<T>*>(m_pSheet);
		return pBindColumn->GetGetter()(pBindGrid->GetItemsSource()[pBindRow->GetIndex<AllTag>() - pBindGrid->GetFrozenCount<RowTag>()]);
	}

	virtual void SetCheck(bool check) override
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		auto pBindColumn = static_cast<CBindCheckBoxColumn<T>*>(m_pColumn);
		auto pBindGrid = static_cast<CBindGridView<T>*>(m_pSheet);
		pBindColumn->GetSetter()(pBindGrid->GetItemsSource()[pBindRow->GetIndex<AllTag>() - pBindGrid->GetFrozenCount<RowTag>()], check);
	}
};