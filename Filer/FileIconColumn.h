#pragma once

#include "ParentMapColumn.h"


class CFileIconColumn:public CParentDefaultMapColumn
{
public:
	CFileIconColumn(CGridView* pGrid);
	virtual ~CFileIconColumn(void){}

	cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

