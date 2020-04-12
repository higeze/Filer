#pragma once
#include "TextCell.h"
#include "DeadlineTimer.h"


/**
 *  Filter Cell
 */
class CFilterCell:public CTextCell
{
private :
	CDeadlineTimer m_deadlinetimer;

public:
	CFilterCell::CFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::LButtonDownEdit){}
	virtual ~CFilterCell() = default;

	virtual std::wstring GetString() override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)override;
};