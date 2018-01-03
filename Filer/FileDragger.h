#pragma once
#include "IMouseObserver.h"
#include "MyPoint.h"
//Pre-Declaration
class CSheet;
struct MouseEventArgs;
struct SetCursorEventArgs;

class CFileDragger:public IMouseObserver
{
private:
	typedef int size_type;
	typedef int coordinates_type;

private:
	CPoint m_ptDragStart;
public:
	CFileDragger():m_ptDragStart(){}
	virtual ~CFileDragger(){}
	
	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);

	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);

	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);


	virtual ISheetState* OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e);


private:

	ISheetState* OnBeginDrag(CSheet* pSheet, MouseEventArgs& e);
	ISheetState* OnDrag(CSheet* pSheet, MouseEventArgs& e);
	ISheetState* OnEndDrag(CSheet* pSheet, MouseEventArgs& e);
private:
	bool IsDragable(CSheet* pSheet, std::pair<size_type, size_type> visibleIndexes);

};
