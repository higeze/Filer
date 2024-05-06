#pragma once
#include "TextCell.h"
#include "named_arguments.h"

class CColumnIndexCell:public CTextCell
{
public:
	static CMenu ContextMenu;
public:
	CColumnIndexCell(CGridView* pSheet,CRow* pRow,CColumn* pColumn)
		:CTextCell(pSheet,pRow,pColumn, arg<"editmode"_s>() = EditMode::ReadOnly){}
	virtual ~CColumnIndexCell() = default;
	virtual std::wstring GetString();
};