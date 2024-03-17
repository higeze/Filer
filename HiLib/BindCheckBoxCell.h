#pragma once
#include "CheckBoxCell.h"
#include "BindCheckBoxColumn.h"

template<typename T>
class CBindCheckBoxCell :public CCheckBoxCell
{
public:
	CBindCheckBoxCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CCheckBoxCell(pSheet, pRow, pColumn, spProperty)
	{
		auto pBindColumn = static_cast<const CBindCheckBoxColumn<T>*>(this->m_pColumn);
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		pBindColumn->GetProperty(pBindRow->GetItem<T>()).binding(m_checkBox.State);
	}
	virtual ~CBindCheckBoxCell() = default;
};