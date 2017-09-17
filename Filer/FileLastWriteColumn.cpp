#include "FileLastWriteColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "PathCell.h"
#include "FileLastWriteCell.h"
#include "Sheet.h"

CFileLastWriteColumn::CFileLastWriteColumn(CGridView* pGrid)
	:CParentDefaultMapColumn(pGrid){}

CFileLastWriteColumn::cell_type CFileLastWriteColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CFileLastWriteColumn::cell_type CFileLastWriteColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"LastWrite");
}

CFileLastWriteColumn::cell_type CFileLastWriteColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CFileLastWriteColumn::cell_type CFileLastWriteColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileLastWriteCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
