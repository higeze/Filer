#include "FileNameColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileNameCell.h"
#include "PathCell.h"

CFileNameColumn::CFileNameColumn(CGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}

CFileNameColumn::cell_type CFileNameColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CFileNameColumn::cell_type CFileNameColumn:: NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Name");
}

CFileNameColumn::cell_type CFileNameColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CFileNameColumn::cell_type CFileNameColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileNameCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}