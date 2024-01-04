#include "BindCheckBoxColumn.h"
#include "SortCell.h"
#include "CheckBoxFilterCell.h"
#include "BindCheckBoxCell.h"
#include "BindGridView.h"


std::shared_ptr<CCell> CBindCheckBoxColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}

std::shared_ptr<CCell> CBindCheckBoxColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = m_header);
}

std::shared_ptr<CCell> CBindCheckBoxColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCheckBoxFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CBindCheckBoxColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CBindCheckBoxCell>(
		m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}