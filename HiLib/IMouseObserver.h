#pragma once

class CSheet;
class ISheetState;
struct MouseEvent;
struct SetCursorEvent;

class IMouseObserver
{
public:
	IMouseObserver(void){}
	virtual ~IMouseObserver(void){}
	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEvent& e)=0;
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEvent& e)=0;
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEvent& e)=0;

	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEvent& e)=0;

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEvent& e)=0;
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEvent& e)=0;

	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEvent& e)=0;

};

