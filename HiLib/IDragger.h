#pragma once
#include "UIElement.h"
class CSheet;

class IDragger
{
public:
	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnBeginDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnEndDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnLeaveDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnPaintDragLine(CSheet* pSheet, const PaintEvent& e) = 0;
};

class CNullDragger:public IDragger
{
	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) { return false; }
	virtual void OnBeginDrag(CSheet* pSheet, const MouseEvent& e) {}
	virtual void OnDrag(CSheet* pSheet, const MouseEvent& e) {}
	virtual void OnEndDrag(CSheet* pSheet, const MouseEvent& e) {}
	virtual void OnLeaveDrag(CSheet* pSheet, const MouseEvent& e) {}
	virtual void OnPaintDragLine(CSheet* pSheet, const PaintEvent& e) {}
};
