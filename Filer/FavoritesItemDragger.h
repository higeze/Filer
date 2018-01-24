#pragma once
#include "Dragger.h"

class CFavoritesItemDragger :public CRowDragger
{
	virtual bool IsTarget(CSheet* pSheet, MouseEventArgs const & e) override{return true;}
	virtual int GetLineRightBottom(CSheet* pSheet) override { return pSheet->LastPointer<RowTag, AllTag>()->GetRightBottom(); }
};
