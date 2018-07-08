#pragma once
#include "TextCell.h"

class CFileLastWriteCell:public CTextCell
{
public:
	CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileLastWriteCell(void){}

	virtual bool IsComparable()const override{return false;}
	virtual string_type GetString() override;
};

