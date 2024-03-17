#include "RowIndexColumn.h"
#include "GridView.h"
#include "Cell.h"
#include "RowIndexCell.h"
#include "CellProperty.h"
#include "D2DWWindow.h"

std::shared_ptr<CCell> CRowIndexColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pGrid,pRow,pColumn,m_pGrid->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pGrid,pRow,pColumn,m_pGrid->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pGrid,pRow,pColumn,m_pGrid->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pGrid,pRow,pColumn,m_pGrid->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CRowIndexCell>(m_pGrid,pRow,pColumn,m_pGrid->GetHeaderProperty());
}

void  CRowIndexColumn::RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc) 
{
	pDirect->FillSolidRectangle(*(m_spCellProperty->NormalFill), rc);
}