#pragma once
#include "HeaderCell.h"
#include "named_arguments.h"

class CRowIndexCell:public CHeaderCell
{
public:
	CRowIndexCell(CGridView* pSheet,CRow* pRow,CColumn* pColumn)
		:CHeaderCell(pSheet,pRow,pColumn,arg<"editmode"_s>() = EditMode::ReadOnly){}
	virtual ~CRowIndexCell() = default;
	virtual std::wstring GetString();
};