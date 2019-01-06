#include "FileIconColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconCell.h"
#include "PathCell.h"

CFileIconColumn::CFileIconColumn(CGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}


CColumn::std::shared_ptr<CCell> CFileIconColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CColumn::std::shared_ptr<CCell> CFileIconColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"I");
}

CColumn::std::shared_ptr<CCell> CFileIconColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CColumn::std::shared_ptr<CCell> CFileIconColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileIconCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}