#include "TaskRow.h"
#include "D2DWWindow.h"
#include "Direct2DWrite.h"

void CTaskRow::OnPaint(const PaintEvent& e)
{
	CRectF rcPaint(m_pSheet->GetCellsRect());
	rcPaint.top = GetTop();
	rcPaint.bottom = GetBottom();

	const MainTask& task = GetItem<MainTask>();
	switch (task.State.get()) {
		case CheckBoxState::True:
			e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(0.5f, 0.5f, 0.5f, 0.5f), rcPaint);
			break;
		case CheckBoxState::Intermediate:
			e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(0.5f, 0.5f, 0.5f, 0.3f), rcPaint);
			break;
		case CheckBoxState::False:
		default:
			if (task.Date == CDate::Now()) {
				e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(1.f, 1.f, 0.f, 0.3f), rcPaint);
			} else if (!task.Date.get().IsInvalid() && task.Date.get() < CDate::Now()) {
				e.WndPtr->GetDirectPtr()->FillSolidRectangle(SolidFill(1.f, 0.f, 0.f, 0.3f), rcPaint);
			} else {
				//Do nothing
			}
			break;
	}
}