#pragma once

#include "ParentMapColumn.h"


class CFileIconColumn:public CParentDefaultMapColumn
{
public:
	CFileIconColumn(CGridView* pGrid = nullptr);
	virtual ~CFileIconColumn(void){}

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;

};

