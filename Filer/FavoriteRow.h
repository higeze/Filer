#pragma once
#include "Row.h"

class CGridView;

class CFavoriteRow:public CRow
{
public:
	CFavoriteRow(CSheet* pSheet, int orderIndex);
	virtual ~CFavoriteRow(){}
	int GetOrderIndex()const;
};

