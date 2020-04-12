#pragma once
#include "TextCell.h"
#include "MyFont.h"

class CFontCell:public CTextCell
{
private:
	d2dw::CFontF m_font;
public:
	CFontCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty, d2dw::CFontF font);
	virtual ~CFontCell(){}
	d2dw::CFontF GetFont();

	void PaintContent(d2dw::CDirect2DWrite* pDirect,d2dw::CRectF rcPaint) override;

	virtual void OnLButtonClk(MouseEvent& e);

	virtual std::wstring GetString();
};
