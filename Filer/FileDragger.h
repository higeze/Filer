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
	
	virtual CSheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);

	virtual CSheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	virtual CSheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);

	virtual CSheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);


	virtual CSheetState* OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	virtual CSheetState* OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e);


private:

	CSheetState* OnBeginDrag(CSheet* pSheet, MouseEventArgs& e);
	CSheetState* OnDrag(CSheet* pSheet, MouseEventArgs& e);
	CSheetState* OnEndDrag(CSheet* pSheet, MouseEventArgs& e);
private:
	bool IsDragable(CSheet* pSheet, std::pair<size_type, size_type> visibleIndexes);

};
