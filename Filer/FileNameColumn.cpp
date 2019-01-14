#include "FileNameColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileNameCell.h"
#include "PathCell.h"
#include "CellProperty.h"

CFileNameColumn::CFileNameColumn(CGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}

std::shared_ptr<CCell> CFileNameColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

std::shared_ptr<CCell> CFileNameColumn:: NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Name");
}

std::shared_ptr<CCell> CFileNameColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileNameColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileNameCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}