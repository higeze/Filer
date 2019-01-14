#include "CompareColumn.h"
#include "CompareColumnHeaderCell.h"
#include "CompareCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "CellProperty.h"

CCompareColumn::CCompareColumn(CGridView* pGrid)
	:CParentDefaultMapColumn(pGrid){}

std::shared_ptr<CCell> CCompareColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCompareColumnHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CCompareColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CCompareColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCompareCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
