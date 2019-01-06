#pragma once

#include "TextCell.h"

class CFileNameCell:public CParameterCell
{
public:
	CFileNameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileNameCell(){}

	virtual bool IsComparable()const override{return false;}
	virtual std::wstring GetString() override;
	void SetStringCore(const std::wstring& str) override;
};