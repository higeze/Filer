#pragma once
#include "TextCell.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"
#include "YearMonthDay.h"
#include "Task.h"

class CTaskMemoCell :public CTextCell
{
public:
	template<typename... Args>
	CTaskMemoCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CTextCell(pSheet, pRow, pColumn, spProperty, args...){}

	virtual ~CTaskMemoCell() = default;

	virtual void PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		auto pBindRow = static_cast<CBindRow*>(m_pRow);
		CYearMonthDay ymd = pBindRow->GetItem<MainTask>().YearMonthDay.get();
		CYearMonthDay now = CYearMonthDay::Now();
		if (ymd == now) {
			pDirect->FillSolidRectangle(SolidFill(1.f,1.f,0.f,0.3f), rcPaint);
		} else if (ymd.IsValid() && ymd < now) {
			pDirect->FillSolidRectangle(SolidFill(1.f,0.f,0.f,0.3f), rcPaint);
		} else {
			pDirect->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);
		}
	}

	virtual bool CanSetStringOnEditing()const override{return false;}

};


