#pragma once
#include "TextCell.h"
#include "MyColor.h"

class CColor;

class CColorCell:public CTextCell
{
private:
	CColor m_color;
public:
	CColorCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CCellProperty> spProperty,CColor color);
	virtual ~CColorCell(){}

	CColor GetColor();
	virtual void PaintBackground(CDC* pDC, CRect rcPaint) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual std::basic_string<TCHAR> GetString() override;
	virtual bool IsComparable()const override{return false;}
};
