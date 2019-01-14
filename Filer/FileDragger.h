#pragma once
#include "UIElement.h"
#include "IDragger.h"
#include "MyPoint.h"
//Pre-Declaration
class CSheet;



class CFileDragger:public IDragger
{
private:
	CPoint m_ptDragStart;
public:
	CFileDragger():m_ptDragStart(-1, -1){}
	virtual ~CFileDragger(){}

public:

	bool IsTarget(CSheet* pSheet, MouseEvent const & e) override;
	void OnBeginDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnEndDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnLeaveDrag(CSheet* pSheet, MouseEvent const & e) override;
	void OnPaintDragLine(CSheet* pSheet, const PaintEvent & e)override{/*Do Nothing*/}

};
