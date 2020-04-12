#pragma once

#include "TextCell.h"
#include "SheetEnums.h"
#include "named_arguments.h"


class CSortCell:public CTextCell
{
private:
	static const FLOAT TRIANGLE_WIDTH;
	static const FLOAT TRIANGLE_HEIGHT;
	static const d2dw::CRectF TRIANGLE_MARGIN;
	static const FLOAT MIN_COLUMN_WIDTH;

public:
	template<typename... Args>
	CSortCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::ReadOnly, args...){}
	virtual ~CSortCell() = default;

	d2dw::CSizeF GetSortSize()const;
	//virtual d2dw::CSizeF GetInitSize(d2dw::CDirect2DWrite& direct) override {return d2dw::CSizeF(MIN_COLUMN_WIDTH,0);}
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* direct) override;
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* direct) override;
	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;

	virtual void OnLButtonClk(const LButtonClkEvent& e)override;

	void PaintSortMark(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint);
};