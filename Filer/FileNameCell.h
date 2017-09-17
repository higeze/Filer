#pragma once

#include "TextCell.h"

class CFileNameCell:public CParameterCell
{
public:
	CFileNameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileNameCell(){}

	virtual bool IsComparable()const override{return false;}
	virtual string_type GetString()const override;
	void SetStringCore(const string_type& str) override;
};