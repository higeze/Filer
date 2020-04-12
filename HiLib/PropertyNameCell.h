#pragma once
#include "TextCell.h"

class CPropertyNameCell:public CTextCell
{
public:
	CPropertyNameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet,pRow,pColumn,spProperty,arg<"text"_s>() = L""){}
	virtual ~CPropertyNameCell() = default;
};