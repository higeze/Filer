#include "FileIconColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconCell.h"
#include "PathCell.h"
#include "CellProperty.h"

CFileIconColumn::CFileIconColumn(CSheet* pSheet)
		:CMapColumn(pSheet){}


std::shared_ptr<CCell> CFileIconColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

std::shared_ptr<CCell> CFileIconColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"I");
}

std::shared_ptr<CCell> CFileIconColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileIconColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileIconCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}