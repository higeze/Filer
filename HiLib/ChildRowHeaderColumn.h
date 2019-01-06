#pragma once

#include "ChildIndexColumn.h"

class CChildRowHeaderColumn:public CChildIndexColumn
{
public:
	CChildRowHeaderColumn(CSheetCell* pSheetCell):CChildIndexColumn(pSheetCell){}
	virtual ~CChildRowHeaderColumn(){}

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};