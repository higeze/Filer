#include "stdafx.h"
#include "FileExtColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "PathCell.h"
#include "FileExtCell.h"
#include "Sheet.h"

CFileExtColumn::cell_type CFileExtColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CFileExtColumn::cell_type CFileExtColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Ext");
}

CFileExtColumn::cell_type CFileExtColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CFileExtColumn::cell_type CFileExtColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileExtCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
