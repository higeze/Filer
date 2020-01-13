#pragma once
#include "Dragger.h"

class CFavoritesItemDragger :public CRowDragger
{
	virtual bool IsTarget(CSheet* pSheet, MouseEvent const & e) override{return true;}
};
