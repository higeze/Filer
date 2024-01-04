#include "BindYearMonthDayColumn.h"
#include "BindGridView.h"
#include "BindYearMonthDayCell.h"
#include "FilterCell.h"
#include "SortCell.h"
#include "Sheet.h"
#include "BindGridView.h"

std::shared_ptr<CCell> CBindYearMonthDayColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}

std::shared_ptr<CCell> CBindYearMonthDayColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = m_header);
}

std::shared_ptr<CCell> CBindYearMonthDayColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CBindYearMonthDayColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CBindYearMonthDayCell>(
		m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}