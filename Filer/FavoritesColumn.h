#pragma once

#include "ParentMapColumn.h"

class CFavorite;


class CFavoritesColumn:public CParentDefaultMapColumn
{
private:
	std::shared_ptr<std::vector<CFavorite>> m_spFavorites;
public:
	CFavoritesColumn(CGridView* pGrid, std::shared_ptr<std::vector<CFavorite>> pFavorites);
	virtual ~CFavoritesColumn(void){}
	//Getter
	std::shared_ptr<std::vector<CFavorite>> GetFavorites() { return m_spFavorites; }


	cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type CellTemplate(CRow* pRow, CColumn* pColumn)override;

	bool IsDragTrackable()const { return true; }
};

