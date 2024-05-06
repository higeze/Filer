#pragma once
#include "BindTextCell.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"
#include "YearMonthDay.h"
#include "Task.h"

class CTaskMemoCell :public CBindTextCell<MainTask>
{
public:
	template<typename... Args>
	CTaskMemoCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, Args... args)
		:CBindTextCell(pSheet, pRow, pColumn, args...){}

	virtual ~CTaskMemoCell() = default;

	virtual void PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rcPaint) override;
	//{
	//	auto pBindRow = static_cast<CBindRow<MainTask>*>(m_pRow);
	//	CYearMonthDay ymd = pBindRow->GetItem<MainTask>().YearMonthDay.get();
	//	CYearMonthDay now = CYearMonthDay::Now();
	//	if (ymd == now) {
	//		pDirect->FillSolidRectangle(SolidFill(1.f,1.f,0.f,0.3f), rcPaint);
	//	} else if (ymd.IsValid() && ymd < now) {
	//		pDirect->FillSolidRectangle(SolidFill(1.f,0.f,0.f,0.3f), rcPaint);
	//	} else {
	//		pDirect->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);
	//	}
	//}

	virtual bool CanSetStringOnEditing()const override{return false;}
};


