#pragma once
#include "CheckBoxCell.h"

class CBindCheckBoxCell :public CCheckBoxCell
{
public:
	CBindCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CBindCheckBoxCell() = default;
};