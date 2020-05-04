#pragma once
#include "CheckBoxCell.h"
#include "BindCheckBoxColumn.h"

template<typename... TItems>
class CBindCheckBoxCell :public CCheckBoxCell
{
public:
	CBindCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CCheckBoxCell(pSheet, pRow, pColumn, spProperty){}
	virtual ~CBindCheckBoxCell() = default;

	virtual CheckBoxState GetCheckBoxState() const override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		auto pBindColumn = static_cast<CBindCheckBoxColumn<TItems...>*>(m_pColumn);
		return pBindColumn->GetGetter()(pBindRow->GetTupleItems());
	}

	virtual void SetCheckBoxState(const CheckBoxState& state) override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		auto pBindColumn = static_cast<CBindCheckBoxColumn<TItems...>*>(m_pColumn);
		pBindColumn->GetSetter()(pBindRow->GetTupleItems(), state);
	}
};