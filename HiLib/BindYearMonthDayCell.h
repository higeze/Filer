#pragma once
#include "TextCell.h"
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

template<typename... TItems>
class CBindYearMonthDayCell :public CTextCell
{
protected:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<CYearMonthDay> YearMonthDay;
	std::wstring m_sortString;
public:
	template<typename... Args>
	CBindYearMonthDayCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::ReadOnly, args...), 
		YearMonthDay(), 
		Dummy(std::make_shared<int>(0))
	{
		YearMonthDay.subscribe([this](const CYearMonthDay& ymd) {
			OnPropertyChanged(L"value");
		}, Dummy);

		auto pBindColumn = static_cast<const CBindYearMonthDayColumn<TItems...>*>(this->m_pColumn);
		auto pBindRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
		pBindColumn->GetProperty(pBindRow->GetTupleItems()).binding(YearMonthDay);
	}

	virtual ~CBindYearMonthDayCell() = default;

	virtual std::wstring GetString() override
	{
		return YearMonthDay->IsInvalid() ? L"" : 
			std::format(L"{:%m/%d}({})", 
			std::chrono::year_month_day(*YearMonthDay),
			YearMonthDay->GetJpnWeekDay());
	}

	virtual std::wstring GetSortString() override
	{
		return YearMonthDay->IsInvalid() ? L"9999-99-99" :
			std::format(L"{:%F}",
			std::chrono::year_month_day(*YearMonthDay));
	}

	virtual void SetString(const std::wstring& str, bool notify = true) override
	{
		YearMonthDay.set(CYearMonthDay::Parse(str));
	}

	virtual bool CanSetStringOnEditing()const override{return false;}

	virtual void OnLButtonClk(const LButtonClkEvent& e) override
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
		m_pSheet->GetGridPtr()->SetFocusedControlPtr(spDlg);
	}


};