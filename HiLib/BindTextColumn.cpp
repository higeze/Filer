#include "BindTextColumn.h"
#include "SortCell.h"
#include "BindTextCell.h"


std::shared_ptr<CCell> CBindTextColumn:: HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}

std::shared_ptr<CCell> CBindTextColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = m_header);
}

std::shared_ptr<CCell> CBindTextColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CBindTextColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CBindTextCell>(
		m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty(),
		arg<"editmode"_s>() = m_cellEditMode);
}
