#pragma once
#include "CheckBoxCell.h"
#include "BindCheckBoxColumn.h"
#include "reactive_binding.h"

template<typename... TItems>
class CBindCheckBoxCell :public CCheckBoxCell
{
public:
	CBindCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CCheckBoxCell(pSheet, pRow, pColumn, spProperty)
	{
		auto pBindColumn = static_cast<const CBindCheckBoxColumn<TItems...>*>(this->m_pColumn);
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		reactive_binding(pBindColumn->GetProperty(pBindRow->GetTupleItems()), m_checkBox.State);
	}
	virtual ~CBindCheckBoxCell() = default;

	//virtual CheckBoxState GetCheckBoxState() const override
	//{
	//	auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
	//	auto pBindColumn = static_cast<const CBindCheckBoxColumn<TItems...>*>(this->m_pColumn);
	//	return pBindColumn->GetGetter()(pBindRow->GetTupleItems());
	//}

	//virtual void SetCheckBoxState(const CheckBoxState& state) override
	//{
	//	auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
	//	auto pBindColumn = static_cast<const CBindCheckBoxColumn<TItems...>*>(this->m_pColumn);
	//	pBindColumn->GetSetter()(pBindRow->GetTupleItems(), state);
	//}
};