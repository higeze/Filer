#include "ChildRowHeaderColumn.h"
#include "ChildHeaderCell.h"
#include "ChildRowHeaderCell.h"
#include "Cell.h"
#include "Sheet.h"
#include "CellProperty.h"

std::shared_ptr<CCell> CChildRowHeaderColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CChildHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CChildRowHeaderColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CChildRowHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}