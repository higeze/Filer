#pragma once
#include "Column.h"

class CChildIndexColumn:public CChildColumn
{
private:
	std::unordered_map<CRow*,std::shared_ptr<CCell>> m_mapCell;
public:
	CChildIndexColumn(CSheetCell* pSheet):CChildColumn(pSheet){}
	virtual ~CChildIndexColumn(){}
	virtual std::shared_ptr<CCell>& Cell(CRow* pRow);
	virtual int GetDataSize()const;
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};