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
	CFilterCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet, pRow, pColumn, arg<"editmode"_s>() = EditMode::LButtonDownEdit){}
	virtual ~CFilterCell() = default;

	virtual std::wstring GetString() override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)override;

	virtual bool CanSetStringOnEditing()const{return true;}
};