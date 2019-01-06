#include "ParentRowHeaderColumn.h"
#include "Sheet.h"
#include "ParentHeaderCell.h"
#include "FilterCell.h"
#include "Cell.h"
#include "ParentRowHeaderCell.h"

CParentRowHeaderColumn::std::shared_ptr<CCell> CParentRowHeaderColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentRowHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

CParentRowHeaderColumn::std::shared_ptr<CCell> CParentRowHeaderColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentRowHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

CParentRowHeaderColumn::std::shared_ptr<CCell> CParentRowHeaderColumn::HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentRowHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

CParentRowHeaderColumn::std::shared_ptr<CCell> CParentRowHeaderColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentRowHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

CParentRowHeaderColumn::std::shared_ptr<CCell> CParentRowHeaderColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentRowHeaderIndexCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}