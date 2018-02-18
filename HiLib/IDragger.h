#pragma once
#include "UIElement.h"
class CSheet;

class IDragger
{
protected:
	typedef int size_type;
	typedef int coordinates_type;
public:
	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnBeginDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnEndDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnLeaveDrag(CSheet* pSheet, const MouseEvent& e) = 0;
	virtual void OnPaintDragLine(CSheet* pSheet, const PaintEvent& e) = 0;
};
