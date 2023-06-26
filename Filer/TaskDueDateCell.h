#pragma once
#include "TextCell.h"
#include "D2DWWindow.h"
#include "ResourceIDFactory.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"
#include "YearMonthDay.h"
#include "Task.h"

class CTaskDueDateCell :public CTextCell
{
public:
	template<typename... Args>
	CTaskDueDateCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CTextCell(pSheet, pRow, pColumn, spProperty, args...){}

	virtual ~CTaskDueDateCell() = default;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<MainTask>*>(m_pRow);
		const CYearMonthDay& ymd = pBindRow->GetItem<MainTask>().YearMonthDay.get();

		return ymd.IsInvalid()? L"" : std::format(L"{}/{}({})", ymd.YearMonthDay.get().month().operator size_t(), ymd.YearMonthDay.get().day().operator size_t(), ymd.GetJpnWeekDay());
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<MainTask>*>(m_pRow);
		pBindRow->GetItem<MainTask>().YearMonthDay.get_unconst().Parse(str);
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
		
		auto pBindRow = static_cast<CBindRow<MainTask>*>(m_pRow);
		MainTask& task = pBindRow->GetItem<MainTask>();
		
		if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Today")) {
			task.YearMonthDay.get_unconst().YearMonthDay.set(CYearMonthDay::Today().YearMonthDay.get());
		} else if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Tomorrow")) {
			task.YearMonthDay.get_unconst().YearMonthDay.set(CYearMonthDay::Tomorrow().YearMonthDay.get());
		}

		*e.HandledPtr = TRUE;

	}


	//virtual void PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rcPaint) override
	//{
	//	auto pBindRow = static_cast<CBindRow<MainTask>*>(m_pRow);
	//	CDate dt = pBindRow->GetItem<MainTask>().Date;
	//	CDate now = CDate::Now();
	//	if (dt == now) {
	//		pDirect->FillSolidRectangle(SolidFill(1.f,1.f,0.f,0.3f), rcPaint);
	//	} else if (!dt.IsInvalid() && dt < now) {
	//		pDirect->FillSolidRectangle(SolidFill(1.f,0.f,0.f,0.3f), rcPaint);
	//	} else {
	//		pDirect->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);
	//	}
	//}
};