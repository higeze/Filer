#pragma once
#include "TextCell.h"
#include "MyColor.h"

class CColorCell:public CTextCell
{
private:
	CColorF m_color;
public:
	CColorCell(CGridView* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,CColorF color);
	virtual ~CColorCell(){}

	CColorF GetColor();
	virtual void PaintBackground(CDirect2DWrite* pDirect, CRectF rcPaint) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual std::basic_string<TCHAR> GetString() override;
};
