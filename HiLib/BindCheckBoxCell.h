#pragma once
#include "CheckBoxCell.h"
#include "BindRow.h"
#include "BindCheckBoxColumn.h"

template<typename T> class CBindRow;
template<typename T> class CBindCheckBoxColumn;

template<typename T>
class CBindCheckBoxCell :public CCheckBoxCell
{
public:
	CBindCheckBoxCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CCheckBoxCell(pSheet, pRow, pColumn)
	{
		auto pBindColumn = static_cast<const CBindCheckBoxColumn<T>*>(this->m_pColumn);
		auto pBindRow = static_cast<CBindRow<T>*>(this->m_pRow);
		pBindColumn->GetProperty(pBindRow->GetItem<T>()).binding(this->m_checkBox.State);
	}
	virtual ~CBindCheckBoxCell() = default;
};