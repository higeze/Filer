#pragma once
#include "TextCell.h"
#include "MyColor.h"

class CColorCell:public CTextCell
{
private:
	d2dw::CColorF m_color;
public:
	CColorCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,d2dw::CColorF color);
	virtual ~CColorCell(){}

	d2dw::CColorF GetColor();
	virtual void PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual std::basic_string<TCHAR> GetString() override;
	virtual bool IsComparable()const override{return false;}
};
