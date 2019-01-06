#pragma once
#include "TextCell.h"

class CFileLastWriteCell:public CTextCell
{
public:
	CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileLastWriteCell(void){}

	virtual bool IsComparable()const override{return false;}
	virtual std::wstring GetString() override;
};

