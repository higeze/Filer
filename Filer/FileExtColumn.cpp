#include "stdafx.h"
#include "FileExtColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "PathCell.h"
#include "FileExtCell.h"
#include "Sheet.h"

CFileExtColumn::std::shared_ptr<CCell> CFileExtColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CFileExtColumn::std::shared_ptr<CCell> CFileExtColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Ext");
}

CFileExtColumn::std::shared_ptr<CCell> CFileExtColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CFileExtColumn::std::shared_ptr<CCell> CFileExtColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileExtCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
