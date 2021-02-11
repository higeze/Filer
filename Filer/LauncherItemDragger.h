#pragma once
#include "Dragger.h"

class CLauncherItemDragger :public CColDragger
{
	virtual bool IsTarget(CSheet* pSheet, MouseEvent const & e) override{return true;}
};
