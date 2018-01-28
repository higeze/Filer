#pragma once
#include "Row.h"

class CGridView;

class CFavoriteRow:public CParentRow
{
public:
	CFavoriteRow(CGridView* pGrid, int orderIndex);
	virtual ~CFavoriteRow(){}
	int GetOrderIndex()const;
};

