#pragma once

class CGridView;
class ISheetState;
struct KeyEvent;

class IKeyObserver
{
public:
	IKeyObserver(void){}
	virtual ~IKeyObserver(void){}
	virtual ISheetState* OnKeyDown(CGridView* pSheet, KeyEvent& e)=0;
};

