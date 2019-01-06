#include "FileNameColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileNameCell.h"
#include "PathCell.h"

CFileNameColumn::CFileNameColumn(CGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}

CFileNameColumn::std::shared_ptr<CCell> CFileNameColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CFileNameColumn::std::shared_ptr<CCell> CFileNameColumn:: NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Name");
}

CFileNameColumn::std::shared_ptr<CCell> CFileNameColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CFileNameColumn::std::shared_ptr<CCell> CFileNameColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileNameCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}