//#pragma once
//#include "SheetState.h"
//
//class CFileDraggingState:public ISheetState
//{
//public:
//	static ISheetState* FileDragging();
//public:
//	CFileDraggingState(){}
//	virtual ~CFileDraggingState(){}
//
//	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
//	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
//	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
//	virtual ISheetState* OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e);
//	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);
//
//	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
//	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
//	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);
//
//};