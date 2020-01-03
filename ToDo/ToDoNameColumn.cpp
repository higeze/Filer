#include "ToDoNameColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "CellProperty.h"


CToDoNameColumn::CToDoNameColumn(CGridView* pGrid)
	:CParentDefaultMapColumn(pGrid){}

std::shared_ptr<CCell> CToDoNameColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentHeaderCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}

std::shared_ptr<CCell> CToDoNameColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), L"Name");
}

std::shared_ptr<CCell> CToDoNameColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CToDoNameColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParameterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}