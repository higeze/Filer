#pragma once

#include "ParentHeaderCell.h"

class CParentColumnHeaderHeaderCell:public CParentHeaderCell
{
public:
	CParentColumnHeaderHeaderCell(CSheet* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty,CMenu* pMenu = nullptr)
		:CParentHeaderCell(pSheet,pRow,pColumn,spProperty,pMenu){}
	virtual ~CParentColumnHeaderHeaderCell(){}
	virtual std::wstring GetString();
	virtual std::wstring GetSortString();
	virtual bool Filter(const std::wstring& strFilter)const;

};