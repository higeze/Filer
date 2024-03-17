#pragma once
#include "UIElement.h"
class CGridView;

class IDragger
{
public:
	virtual bool IsTarget(CGridView* pSheet, const MouseEvent& e) = 0;
	virtual void OnBeginDrag(CGridView* pSheet, const MouseEvent& e) = 0;
	virtual void OnDrag(CGridView* pSheet, const MouseEvent& e) = 0;
	virtual void OnEndDrag(CGridView* pSheet, const MouseEvent& e) = 0;
	virtual void OnLeaveDrag(CGridView* pSheet, const MouseEvent& e) = 0;
	virtual void OnPaintDragLine(CGridView* pSheet, const PaintEvent& e) = 0;
};

class CNullDragger:public IDragger
{
	virtual bool IsTarget(CGridView* pSheet, const MouseEvent& e) { return false; }
	virtual void OnBeginDrag(CGridView* pSheet, const MouseEvent& e) {}
	virtual void OnDrag(CGridView* pSheet, const MouseEvent& e) {}
	virtual void OnEndDrag(CGridView* pSheet, const MouseEvent& e) {}
	virtual void OnLeaveDrag(CGridView* pSheet, const MouseEvent& e) {}
	virtual void OnPaintDragLine(CGridView* pSheet, const PaintEvent& e) {}
};
