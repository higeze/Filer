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

template<typename T>
class CBindYearMonthDayCell :public CTextCell
{
protected:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<CYearMonthDay> YearMonthDay;
	std::wstring m_sortString;
public:
	template<typename... Args>
	CBindYearMonthDayCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, Args... args)
		:CTextCell(pSheet, pRow, pColumn, arg<"editmode"_s>() = EditMode::ReadOnly, args...), 
		YearMonthDay(), 
		Dummy(std::make_shared<int>(0))
	{
		YearMonthDay.subscribe([this](const CYearMonthDay& ymd) {
			OnPropertyChanged(L"value");
		}, Dummy);

		auto pBindColumn = static_cast<const CBindYearMonthDayColumn<T>*>(this->m_pColumn);
		auto pBindRow = static_cast<CBindRow<T>*>(this->m_pRow);
		pBindColumn->GetProperty(pBindRow->GetItem<T>()).binding(YearMonthDay);
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
		auto spDlg = std::make_shared<CCalendarDialog>(m_pGrid);

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
			std::clamp(e.PointInWnd.x, m_pGrid->GetRectInWnd().left, m_pGrid->GetRectInWnd().right - spDlg->DesiredSize().width),
			std::clamp(e.PointInWnd.y, m_pGrid->GetRectInWnd().top, m_pGrid->GetRectInWnd().bottom - spDlg->DesiredSize().height)
		);
		spDlg->OnCreate(CreateEvt(m_pGrid->GetWndPtr(), m_pGrid, CRectF(pt, spDlg->DesiredSize())));
		spDlg->Arrange(CRectF(pt, spDlg->DesiredSize()));
		m_pGrid->GetWndPtr()->SetFocusToControl(spDlg);
	}


};