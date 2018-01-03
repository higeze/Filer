#pragma once

class CGridView;
class ISheetState;
struct KeyEventArgs;

class IKeyObserver
{
public:
	IKeyObserver(void){}
	virtual ~IKeyObserver(void){}
	virtual ISheetState* OnKeyDown(CGridView* pSheet, KeyEventArgs& e)=0;
};

