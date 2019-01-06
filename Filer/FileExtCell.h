#pragma once

#include "TextCell.h"

class CFileExtCell:public CParameterCell
{
public:
	CFileExtCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileExtCell(void){}

	virtual bool IsComparable()const override{return false;}
	virtual std::wstring GetString() override;
	void SetStringCore(const std::wstring& str) override;
};

