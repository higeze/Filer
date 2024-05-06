#pragma once
#include "Cell.h"

class CExpandCollapseCell:public CCell
{
private:
	bool m_isExpanded;
public:
	CExpandCollapseCell(CGridView* pSheet,CRow* pRow, CColumn* pColumn)
		:CCell(pSheet, pRow, pColumn), m_isExpanded(true){}
	virtual ~CExpandCollapseCell(){}
};