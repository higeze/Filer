#pragma once
#include "CheckBoxCell.h"
#include "BindCheckBoxColumn.h"

template<typename TItem>
class CBindCheckBoxCell :public CCheckBoxCell
{
public:
	CBindCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CCheckBoxCell(pSheet, pRow, pColumn, spProperty){}
	virtual ~CBindCheckBoxCell() = default;

	virtual CheckBoxState GetCheckBoxState() const override
	{
		auto pBindRow = static_cast<CBindRow<TItem>*>(m_pRow);
		auto pBindColumn = static_cast<CBindCheckBoxColumn<TItem>*>(m_pColumn);
		return pBindColumn->GetGetter()(pBindRow->GetItem());
	}

	virtual void SetCheckBoxState(const CheckBoxState& state) override
	{
		auto pBindRow = static_cast<CBindRow<TItem>*>(m_pRow);
		auto pBindColumn = static_cast<CBindCheckBoxColumn<TItem>*>(m_pColumn);
		pBindColumn->GetSetter()(pBindRow->GetItem(), state);
	}
};