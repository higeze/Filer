#pragma once

#include "TextCell.h"

class CChildHeaderCell:public CTextCell
{
public:
	CChildHeaderCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet,pRow, pColumn,spProperty){}
	virtual ~CChildHeaderCell(){}
};