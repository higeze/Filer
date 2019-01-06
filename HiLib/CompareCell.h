#pragma once

#include "TextCell.h"

class CCompareCell:public CTextCell
{
public:
	CCompareCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet,pRow,pColumn,spProperty){}
	virtual ~CCompareCell(){}

	virtual bool IsComparable()const
	{
		return false;
	}

	virtual std::wstring GetString();
};