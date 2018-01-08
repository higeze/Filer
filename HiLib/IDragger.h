#pragma once

class CSheet;
struct MouseEventArgs;
struct PaintEventArgs;

class IDragger
{
protected:
	typedef int size_type;
	typedef int coordinates_type;
public:
	virtual bool IsTarget(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnBeginDrag(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnDrag(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnEndDrag(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnLeaveDrag(CSheet* pSheet, MouseEventArgs const & e) = 0;
	virtual void OnPaintDragLine(CSheet* pSheet, PaintEventArgs const & e) = 0;
};
