#pragma once
#include "FilerGridView.h"

class CFilerGridViewWnd :public CFilerGridView
{
public:
	//Constructor
	CFilerGridViewWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp);
	virtual ~CFilerGridViewWnd() {}
};