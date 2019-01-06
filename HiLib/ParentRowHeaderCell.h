#pragma once

#include "ParentHeaderCell.h"

class CParentRowHeaderCell:public CParentHeaderCell
{
public:
	static CMenu ContextMenu;
public:
	CParentRowHeaderCell(CSheet* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty,CMenu* pMenu = &CParentRowHeaderCell::ContextMenu)
		:CParentHeaderCell(pSheet,pRow,pColumn,spProperty,pMenu?pMenu:&CParentRowHeaderCell::ContextMenu){}
	virtual ~CParentRowHeaderCell(){}
	virtual std::wstring GetString();
	virtual std::wstring GetSortString();
	virtual bool Filter(const std::wstring& strFilter)const;
};

class CParentRowHeaderIndexCell:public CParentRowHeaderCell
{
public:
	CParentRowHeaderIndexCell(CSheet* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty,CMenu* pMenu = nullptr)
		:CParentRowHeaderCell(pSheet,pRow,pColumn,spProperty,pMenu){}
	virtual ~CParentRowHeaderIndexCell(){}
	virtual std::wstring GetString();
};