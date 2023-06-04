#pragma once
#include "TextCell.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"
#include "Date.h"
#include "Task.h"

class CDateCell :public CTextCell
{
public:
	template<typename... Args>
	CDateCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CTextCell(pSheet, pRow, pColumn, spProperty, args...){}

	virtual ~CDateCell() = default;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<MainTask>*>(m_pRow);
		const CDate& dt = pBindRow->GetItem<MainTask>().Date;

		return dt.IsInvalid()? L"" : std::format(L"{}/{}({})", dt.GetMonth(), dt.GetDay(), dt.GetJpnWeekDay());
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<MainTask>*>(m_pRow);
		pBindRow->GetItem<MainTask>().Date.Parse(str);
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