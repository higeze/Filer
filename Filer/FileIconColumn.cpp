#include "FileIconColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconCell.h"
#include "PathCell.h"

CFileIconColumn::CFileIconColumn(CGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}


CColumn::cell_type CFileIconColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CColumn::cell_type CFileIconColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"I");
}

CColumn::cell_type CFileIconColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CColumn::cell_type CFileIconColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileIconCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}