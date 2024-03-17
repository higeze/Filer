#pragma once

class CGridView;
class ISheetState;
struct MouseEvent;
struct SetCursorEvent;

class IMouseObserver
{
public:
	IMouseObserver(void){}
	virtual ~IMouseObserver(void){}
	virtual ISheetState* OnLButtonDown(CGridView* pSheet, MouseEvent& e)=0;
	virtual ISheetState* OnLButtonUp(CGridView* pSheet, MouseEvent& e)=0;
	virtual ISheetState* OnLButtonDblClk(CGridView* pSheet, MouseEvent& e)=0;

	virtual ISheetState* OnRButtonDown(CGridView* pSheet, MouseEvent& e)=0;

	virtual ISheetState* OnMouseMove(CGridView* pSheet, MouseEvent& e)=0;
	virtual ISheetState* OnMouseLeave(CGridView* pSheet, MouseEvent& e)=0;

	virtual ISheetState* OnSetCursor(CGridView* pSheet, SetCursorEvent& e)=0;

};

