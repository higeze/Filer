#pragma once
#include "UIElement.h"
#include "IDragger.h"
#include "MyPoint.h"
//Pre-Declaration
class CSheet;

class CFileDragger:public IDragger
{
private:
	typedef int size_type;
	typedef int coordinates_type;

private:
	CPoint m_ptDragStart;
public:
	CFileDragger():m_ptDragStart(-1, -1){}
	virtual ~CFileDragger(){}
	
	//virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);

	//virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	//virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);

	//virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);


	//virtual ISheetState* OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	//virtual ISheetState* OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e);


public:

	bool IsTarget(CSheet* pSheet, MouseEvent const & e) override;
	void OnBeginDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnEndDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnLeaveDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnPaintDragLine(CSheet* pSheet, const PaintEvent & e)override{/*Do Nothing*/}

};
