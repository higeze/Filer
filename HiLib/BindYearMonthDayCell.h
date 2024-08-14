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
		this->YearMonthDay.subscribe([this](const CYearMonthDay& ymd) {
			OnPropertyChanged(L"value");
		}, this->Dummy);

		auto pBindColumn = static_cast<const CBindYearMonthDayColumn<T>*>(this->m_pColumn);
		auto pBindRow = static_cast<CBindRow<T>*>(this->m_pRow);
		pBindColumn->GetProperty(pBindRow->GetItem<T>()).binding(this->YearMonthDay);
	}

	virtual ~CBindYearMonthDayCell() = default;

	virtual std::wstring GetString() override
	{
		return this->YearMonthDay->IsInvalid() ? L"" : 
			std::format(L"{:%m/%d}({})", 
			std::chrono::year_month_day(*this->YearMonthDay),
			this->YearMonthDay->GetJpnWeekDay());
	}

	virtual std::wstring GetSortString() override
	{
		return this->YearMonthDay->IsInvalid() ? L"9999-99-99" :
			std::format(L"{:%F}",
			std::chrono::year_month_day(*this->YearMonthDay));
	}

	virtual void SetString(const std::wstring& str, bool notify = true) override
	{
		this->YearMonthDay.set(CYearMonthDay::Parse(str));
	}

	virtual bool CanSetStringOnEditing()const override{return false;}

	virtual void OnLButtonClk(const LButtonClkEvent& e) override
	{
		auto spDlg = std::make_shared<CCalendarDialog>(this->m_pGrid);

		if (this->YearMonthDay->IsValid()) {
			spDlg->GetCalendarPtr()->Year.set(this->YearMonthDay->year());
			spDlg->GetCalendarPtr()->Month.set(this->YearMonthDay->month());
		} else {
			auto today = CYearMonthDay::Today();
			spDlg->GetCalendarPtr()->Year.set(today.year());
			spDlg->GetCalendarPtr()->Month.set(today.month());
		}
		this->YearMonthDay.binding(spDlg->GetCalendarPtr()->SelectedYearMonthDay);

		spDlg->Measure(CSizeF(300, 200));
		CPointF pt(
			std::clamp(e.PointInWnd.x, this->m_pGrid->GetRectInWnd().left, this->m_pGrid->GetRectInWnd().right - spDlg->DesiredSize().width),
			std::clamp(e.PointInWnd.y, this->m_pGrid->GetRectInWnd().top, this->m_pGrid->GetRectInWnd().bottom - spDlg->DesiredSize().height)
		);
		spDlg->OnCreate(CreateEvt(this->m_pGrid->GetWndPtr(), this->m_pGrid, CRectF(pt, spDlg->DesiredSize())));
		spDlg->Arrange(CRectF(pt, spDlg->DesiredSize()));
		this->m_pGrid->GetWndPtr()->SetFocusToControl(spDlg);
	}


};