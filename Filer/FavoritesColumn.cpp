#include "FavoritesColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconStringCell.h"
#include "PathCell.h"

CFavoritesColumn::CFavoritesColumn(CGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}


CColumn::cell_type CFavoritesColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());	
}

CColumn::cell_type CFavoritesColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Icon");
}

CColumn::cell_type CFavoritesColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CColumn::cell_type CFavoritesColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileIconStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
