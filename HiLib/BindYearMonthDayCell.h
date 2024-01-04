#pragma once
#include "TextCell.h"
#include "BindRow.h"
#include "BindYearMonthDayColumn.h"
#include "YearMonthDay.h"
#include "reactive_property.h"

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

		auto pBindColumn = static_cast<const CBindYearMonthDayColumn*>(this->m_pColumn);
		auto pBindRow = static_cast<CBindRow*>(this->m_pRow);
		pBindColumn->GetProperty(pBindRow->GetTupleItems()).binding(YearMonthDay);
	}

	virtual ~CBindYearMonthDayCell() = default;

	virtual std::wstring GetString() override;

	virtual std::wstring GetSortString() override;

	virtual void SetString(const std::wstring& str, bool notify = true) override;

	virtual bool CanSetStringOnEditing()const override{return false;}

	virtual void OnLButtonClk(const LButtonClkEvent& e) override;


};