#pragma once
#include "SheetState.h"

class CFileDraggingState:public CSheetState
{
public:
	static CSheetState* FileDragging();
public:
	CFileDraggingState(){}
	virtual ~CFileDraggingState(){}

	virtual CSheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	virtual CSheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);

};