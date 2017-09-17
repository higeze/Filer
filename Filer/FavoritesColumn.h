#pragma once

#include "ParentMapColumn.h"


class CFavoritesColumn:public CParentDefaultMapColumn
{
public:
	CFavoritesColumn(CGridView* pGrid);
	virtual ~CFavoritesColumn(void){}

	cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

