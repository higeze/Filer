#pragma once
#include "TextCell.h"
#include "MyFont.h"

class CFontCell:public CTextCell
{
private:
	CFontF m_font;
public:
	CFontCell(CGridView* pSheet,CRow* pRow, CColumn* pColumn,CFontF font);
	virtual ~CFontCell(){}
	CFontF GetFont();

	void PaintContent(CDirect2DWrite* pDirect,CRectF rcPaint) override;

	virtual void OnLButtonClk(MouseEvent& e);

	virtual std::wstring GetString();
};
