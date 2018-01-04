#pragma once

class CSheet;
struct MouseEventArgs;

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
	virtual size_type GetDragToAllIndex() = 0;//TODO
	virtual size_type GetDragFromAllIndex() = 0;//TODO
};
