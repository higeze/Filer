#include "TaskRow.h"
#include "D2DWWindow.h"
#include "Direct2DWrite.h"

void CTaskRow::RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc)
{
	const MainTask& task = GetItem<MainTask>();
	switch (*task.State) {
		case CheckBoxState::True:
			pDirect->FillSolidRectangle(SolidFill(0.5f, 0.5f, 0.5f, 0.5f), rc);
			break;
		case CheckBoxState::Intermediate:
			pDirect->FillSolidRectangle(SolidFill(0.f, 0.f, 1.f, 0.3f), rc);
			break;
		case CheckBoxState::False:
		default:
			if (task.YearMonthDay->IsInvalid()) {
				//Transparent
			} else {
				static SolidFill today(1.f, 1.f, 0.f, 0.3f);
				static SolidFill tomorrow(1.f, 1.f, 0.8f, 0.3f);
				static SolidFill passed(1.f, 0.f, 0.f, 0.3f);
				if (CYearMonthDay::Today() == *task.YearMonthDay) {
					pDirect->FillSolidRectangle(today, rc);
				} else if(CYearMonthDay::Tomorrow() == *task.YearMonthDay){
					pDirect->FillSolidRectangle(tomorrow, rc);
				} else if(CYearMonthDay::Today() > *task.YearMonthDay){
					pDirect->FillSolidRectangle(passed, rc);
				} else {
					//Transparent
				}
			}
			//Gradation
			//if (task.YearMonthDay->get_const() == CYearMonthDay::Now()) {
			//	pDirect->FillSolidRectangle(SolidFill(1.f, 1.f, 0.f, 0.3f), rc);
			//} else if (task.YearMonthDay->get_const().IsValid() && task.YearMonthDay->get_const() < CYearMonthDay::Now()) {
			//	pDirect->FillSolidRectangle(SolidFill(1.f, 0.f, 0.f, 0.3f), rc);
			//} else {
			//	//Do nothing
			//}
			break;
	}
}