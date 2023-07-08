#include "TaskRow.h"
#include "D2DWWindow.h"
#include "Direct2DWrite.h"

void CTaskRow::OnPaint(const PaintEvent& e)
{
	CRectF rcPaint(m_pSheet->GetCellsRect());
	rcPaint.top = GetTop();
	rcPaint.bottom = GetBottom();

	const MainTask& task = GetItem<MainTask>();
	switch (task.State->get_const()) {
		case CheckBoxState::True:
			e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(0.5f, 0.5f, 0.5f, 0.5f), rcPaint);
			break;
		case CheckBoxState::Intermediate:
			e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(0.f, 0.f, 1.f, 0.3f), rcPaint);
			break;
		case CheckBoxState::False:
		default:
			if (task.YearMonthDay->get_const() == CYearMonthDay::Now()) {
				e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(1.f, 1.f, 0.f, 0.3f), rcPaint);
			} else if (task.YearMonthDay->get_const().IsValid() && task.YearMonthDay->get_const() < CYearMonthDay::Now()) {
				e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(1.f, 0.f, 0.f, 0.3f), rcPaint);
			} else {
				//Do nothing
			}
			break;
	}
}