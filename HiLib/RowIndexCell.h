#pragma once
#include "ParentHeaderCell.h"
#include "named_arguments.h"

class CRowIndexCell:public CTextCell
{
public:
	CRowIndexCell(CGridView* pSheet,CRow* pRow,CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet,pRow,pColumn,spProperty,arg<"editmode"_s>() = EditMode::ReadOnly){}
	virtual ~CRowIndexCell() = default;
	virtual std::wstring GetString();
};