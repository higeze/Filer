#include "FavoritesColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FavoriteCell.h"
#include "PathCell.h"

CFavoritesColumn::CFavoritesColumn(CGridView* pGrid, std::shared_ptr<std::vector<CFavorite>> pFavorites)
		:CParentDefaultMapColumn(pGrid),m_spFavorites(pFavorites){}


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
	return std::make_shared<CFavoriteCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
