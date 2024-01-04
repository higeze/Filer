#include "BindYearMonthDayCell.h"
#include "D2DWWindow.h"
#include "ResourceIDFactory.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindYearMonthDayColumn.h"
#include "YearMonthDay.h"
#include "reactive_property.h"
#include "CalendarDialog.h"
#include "CalendarControl.h"
#include "DialogProperty.h"
#include "TextBoxProperty.h"
#include "ButtonProperty.h"


std::wstring CBindYearMonthDayCell::GetString()
{
	return YearMonthDay->IsInvalid() ? L"" : 
		std::format(L"{:%m/%d}({})", 
		std::chrono::year_month_day(*YearMonthDay),
		YearMonthDay->GetJpnWeekDay());
}

std::wstring CBindYearMonthDayCell::GetSortString()
{
	return YearMonthDay->IsInvalid() ? L"9999-99-99" :
		std::format(L"{:%F}",
		std::chrono::year_month_day(*YearMonthDay));
}

void CBindYearMonthDayCell::SetString(const std::wstring& str, bool notify)
{
	YearMonthDay.set(CYearMonthDay::Parse(str));
}

void CBindYearMonthDayCell::OnLButtonClk(const LButtonClkEvent& e)
{
	auto spDlg = std::make_shared<CCalendarDialog>(
		m_pSheet->GetGridPtr(),
		std::make_shared<DialogProperty>(),
		std::make_shared<CalendarControlProperty>(),
		std::make_shared<TextBoxProperty>(),
		std::make_shared<ButtonProperty>());

	if (YearMonthDay->IsValid()) {
		spDlg->GetCalendarPtr()->Year.set(YearMonthDay->year());
		spDlg->GetCalendarPtr()->Month.set(YearMonthDay->month());
	} else {
		auto today = CYearMonthDay::Today();
		spDlg->GetCalendarPtr()->Year.set(today.year());
		spDlg->GetCalendarPtr()->Month.set(today.month());
	}
	YearMonthDay.binding(spDlg->GetCalendarPtr()->SelectedYearMonthDay);

	spDlg->Measure(CSizeF(300, 200));
	CPointF pt(
		std::clamp(e.PointInWnd.x, m_pSheet->GetRectInWnd().left, m_pSheet->GetRectInWnd().right - spDlg->DesiredSize().width),
		std::clamp(e.PointInWnd.y, m_pSheet->GetRectInWnd().top, m_pSheet->GetRectInWnd().bottom - spDlg->DesiredSize().height)
	);
	spDlg->OnCreate(CreateEvt(m_pSheet->GetWndPtr(), m_pSheet->GetGridPtr(), CRectF(pt, spDlg->DesiredSize())));
	spDlg->Arrange(CRectF(pt, spDlg->DesiredSize()));
	m_pSheet->GetWndPtr()->SetFocusToControl(spDlg);
}
