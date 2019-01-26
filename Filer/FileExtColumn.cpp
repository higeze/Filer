//#include "stdafx.h"
#include "FileExtColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "PathCell.h"
#include "FileExtCell.h"
#include "Sheet.h"
#include "CellProperty.h"

std::shared_ptr<CCell> CFileExtColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

std::shared_ptr<CCell> CFileExtColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Ext");
}

std::shared_ptr<CCell> CFileExtColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileExtColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileExtCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
