#pragma once

#include "TextCell.h"
#include "SheetEnums.h"
#include "named_arguments.h"


class CSortCell:public CTextCell
{
private:
	static const FLOAT TRIANGLE_WIDTH;
	static const FLOAT TRIANGLE_HEIGHT;
	static const CRectF TRIANGLE_MARGIN;
	static const FLOAT MIN_COLUMN_WIDTH;

public:
	template<typename... Args>
	CSortCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::ReadOnly, args...){}
	virtual ~CSortCell() = default;

	CSizeF GetSortSize()const;
	//virtual CSizeF GetInitSize(CDirect2DWrite& direct) override {return CSizeF(MIN_COLUMN_WIDTH,0);}
	virtual CSizeF MeasureContentSize(CDirect2DWrite* direct) override;
	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* direct) override;
	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;

	virtual void OnLButtonClk(const LButtonClkEvent& e)override;

	void PaintSortMark(CDirect2DWrite* pDirect, CRectF rcPaint);
};