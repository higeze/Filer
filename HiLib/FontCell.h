#pragma once
#include "TextCell.h"
#include "MyFont.h"

class CFontCell:public CTextCell
{
private:
	d2dw::Font m_font;
public:
	CFontCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty, d2dw::Font font);
	virtual ~CFontCell(){}
	d2dw::Font GetFont();

	void PaintContent(d2dw::CDirect2DWrite& direct,d2dw::CRectF rcPaint) override;

	virtual void OnLButtonClk(MouseEvent& e);

	virtual std::wstring GetString();

	virtual bool IsComparable()const{return false;}
};
