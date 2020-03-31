#pragma once

#include "MapColumn.h"


class CFileIconColumn:public CMapColumn
{
public:
	CFileIconColumn(CSheet* pSheet = nullptr);
	virtual ~CFileIconColumn(void){}

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;

};

