#pragma once
#include "UIElement.h"
#include "IDragger.h"
#include "MyPoint.h"
//Pre-Declaration
class CGridView;



class CFileDragger:public IDragger
{
private:
	CPoint m_ptDragStart;
public:
	CFileDragger():m_ptDragStart(-1, -1){}
	virtual ~CFileDragger(){}

public:

	bool IsTarget(CGridView* pSheet, MouseEvent const & e) override;
	void OnBeginDrag(CGridView* pSheet, MouseEvent const & e) override;
	void OnDrag(CGridView* pSheet, MouseEvent const & e) override;
	void OnEndDrag(CGridView* pSheet, MouseEvent const & e) override;
	void OnLeaveDrag(CGridView* pSheet, MouseEvent const & e) override;
	void OnPaintDragLine(CGridView* pSheet, const PaintEvent & e)override{/*Do Nothing*/}

};
