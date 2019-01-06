#pragma once
#include "ChildHeaderCell.h"

class CChildRowHeaderCell:public CChildHeaderCell
{
public:
	CChildRowHeaderCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty)
		:CChildHeaderCell(pSheet,pRow, pColumn,spProperty){}
	virtual ~CChildRowHeaderCell(){}
};