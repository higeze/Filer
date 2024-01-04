#include "BindCheckBoxCell.h"
#include "BindCheckBoxColumn.h"
#include "BindGridView.h"

CBindCheckBoxCell::CBindCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CCheckBoxCell(pSheet, pRow, pColumn, spProperty)
{
	auto pBindColumn = static_cast<const CBindCheckBoxColumn*>(this->m_pColumn);
	auto pBindRow = static_cast<CBindRow*>(m_pRow);
	pBindColumn->GetProperty(pBindRow->GetTupleItems()).binding(m_checkBox.State);
}
