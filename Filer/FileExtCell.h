#pragma once

#include "TextCell.h"

class CFileExtCell:public CTextCell
{
public:
	CFileExtCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileExtCell(void){}

	virtual bool IsComparable()const override{return false;}
	virtual string_type GetString()const override;
};

