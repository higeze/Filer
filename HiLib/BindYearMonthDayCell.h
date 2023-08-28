#pragma once
#include "TextCell.h"
#include "D2DWWindow.h"
#include "ResourceIDFactory.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindYearMonthDayColumn.h"
#include "YearMonthDay.h"
#include "reactive_property.h"

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
		:CTextCell(pSheet, pRow, pColumn, spProperty, args...), YearMonthDay(), Dummy(std::make_shared<int>(0))
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

	virtual void OnContextMenu(const ContextMenuEvent& e) override
	{
		CMenu menu(::CreatePopupMenu());
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_STRING;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Today");
		mii.dwTypeData = const_cast<LPWSTR>(L"Today");
		mii.cch = 4;
		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Tomorrow");
		mii.dwTypeData = const_cast<LPWSTR>(L"Tomorrow");
		mii.cch = 6;
		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);


		::SetForegroundWindow(this->m_pSheet->GetWndPtr()->m_hWnd);
		WORD retID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, e.PointInScreen.x, e.PointInScreen.y, this->m_pSheet->GetWndPtr()->m_hWnd);
		
		if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Today")) {
			YearMonthDay.set(CYearMonthDay::Today());
		} else if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Tomorrow")) {
			YearMonthDay.set(CYearMonthDay::Tomorrow());
		}

		*e.HandledPtr = TRUE;

	}
};