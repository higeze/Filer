#pragma once
#include "TextCell.h"
#include "DeadlineTimer.h"


/**
 *  Filter Cell
 */
class CFilterCell:public CEditableCell
{
private :
	CDeadlineTimer m_deadlinetimer;

public:
	CFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu = nullptr);
	virtual ~CFilterCell();

	virtual std::wstring GetString() override;
	virtual void SetString(const std::wstring& str)override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)override;
	virtual bool IsComparable()const override
	{
		return false;
	}
};