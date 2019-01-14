#include "FileIconColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconCell.h"
#include "PathCell.h"
#include "CellProperty.h"

CFileIconColumn::CFileIconColumn(CGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}


std::shared_ptr<CCell> CFileIconColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

std::shared_ptr<CCell> CFileIconColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"I");
}

std::shared_ptr<CCell> CFileIconColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileIconColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileIconCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}