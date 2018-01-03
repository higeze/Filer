#pragma once

class CSheet;
class ISheetState;
struct MouseEventArgs;
struct SetCursorEventArgs;

class IMouseObserver
{
public:
	IMouseObserver(void){}
	virtual ~IMouseObserver(void){}
	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)=0;
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)=0;
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)=0;

	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)=0;

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e)=0;
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)=0;

	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)=0;

};

