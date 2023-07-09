#include "RowIndexColumn.h"
#include "Sheet.h"
#include "Cell.h"
#include "RowIndexCell.h"
#include "CellProperty.h"
#include "D2DWWindow.h"

std::shared_ptr<CCell> CRowIndexColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CRowIndexColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CRowIndexCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

void CRowIndexColumn::OnPaint(const PaintEvent& e)
{
	CRectF rcPaint(GetRectInWnd());
	e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);
}