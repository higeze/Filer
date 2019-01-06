#pragma once

#include "ParentHeaderCell.h"

class CParentColumnHeaderCell:public CParentHeaderCell
{
public:
	static CMenu ContextMenu;
public:
	CParentColumnHeaderCell(CSheet* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty,CMenu* pMenu = &CParentColumnHeaderCell::ContextMenu)
		:CParentHeaderCell(pSheet,pRow,pColumn,spProperty,pMenu?pMenu:&CParentColumnHeaderCell::ContextMenu){}
	virtual ~CParentColumnHeaderCell(){}
	virtual std::wstring GetString();
	virtual std::wstring GetSortString();
	virtual bool Filter(const std::wstring& strFilter)const;

};

class CParentColumnHeaderIndexCell:public CParentColumnHeaderCell
{
public:
	CParentColumnHeaderIndexCell(CSheet* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty,CMenu* pMenu = nullptr)
		:CParentColumnHeaderCell(pSheet,pRow,pColumn,spProperty,pMenu){}
	virtual ~CParentColumnHeaderIndexCell(){}
	virtual std::wstring GetString();
};

class CParentDefaultColumnHeaderIndexCell:public CParentColumnHeaderIndexCell
{
public:
	static CMenu ContextMenu;
public:
	CParentDefaultColumnHeaderIndexCell(CSheet* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty,CMenu* pMenu = &CParentDefaultColumnHeaderIndexCell::ContextMenu)
		:CParentColumnHeaderIndexCell(pSheet,pRow,pColumn,spProperty,pMenu?pMenu:&CParentDefaultColumnHeaderIndexCell::ContextMenu){}
	virtual ~CParentDefaultColumnHeaderIndexCell(){}
};