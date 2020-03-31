#include "FavoritesColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FavoriteCell.h"
#include "PathCell.h"
#include "CellProperty.h"

CFavoritesColumn::CFavoritesColumn(CSheet* pSheet, std::shared_ptr<std::vector<std::shared_ptr<CFavorite>>> pFavorites)
		:CMapColumn(pSheet),m_spFavorites(pFavorites)
{
	m_isMinLengthFit = true;
}


std::shared_ptr<CCell> CFavoritesColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());	
}

std::shared_ptr<CCell> CFavoritesColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Icon");
}

std::shared_ptr<CCell> CFavoritesColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFavoritesColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFavoriteCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
