#pragma once
#include "TextCell.h"
#include "named_arguments.h"

class CColumnIndexCell:public CTextCell
{
public:
	static CMenu ContextMenu;
public:
	CColumnIndexCell(CSheet* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet,pRow,pColumn,spProperty, arg<"editmode"_s>() = EditMode::ReadOnly){}
	virtual ~CColumnIndexCell() = default;
	virtual std::wstring GetString();
};