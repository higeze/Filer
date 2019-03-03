#pragma once

#include "ParentMapColumn.h"

class CFavorite;


class CFavoritesColumn:public CParentDefaultMapColumn
{
private:
	std::shared_ptr<std::vector<std::shared_ptr<CFavorite>>> m_spFavorites;
public:
	CFavoritesColumn(CGridView* pGrid, std::shared_ptr<std::vector<std::shared_ptr<CFavorite>>> pFavorites);
	virtual ~CFavoritesColumn(void){}
	//Getter
	std::shared_ptr<std::vector<std::shared_ptr<CFavorite>>> GetFavorites() { return m_spFavorites; }


	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;

	bool IsDragTrackable()const { return true; }
};

