#pragma once
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "D2DWWindow.h"
#include "D2DWTypes.h"
#include "YearMonthDay.h"
#include <chrono>
#include "reactive_property.h"
#include "reactive_command.h"

//struct CalendarControlProperty
//{
//	CRectF Padding = CRectF(2,2,2,2);
//	SolidLine Border = SolidLine(0.f, 0.f, 0.f, 1.f, 1.f);
//	SolidFill TodayFill = SolidFill(0.f, 1.f, 1.f, 0.2f);
//	SolidLine SelectedLine = SolidLine(22.f / 255.f, 160.f / 255.f, 133.f / 255.f, 1.0f, 2.0f);
//
//	FormatF Format = FormatF(CFontF(L"Meiryo", CDirect2DWrite::Points2Dips(9)), CColorF(0.0f, 0.0f, 0.0f, 1.0f), 
//		CAlignmentF(DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
//	FormatF SaturdayFormat = FormatF(CFontF(L"Meiryo", CDirect2DWrite::Points2Dips(9)), CColorF(0.0f, 0.0f, 1.0f, 1.0f), 
//		CAlignmentF(DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
//	FormatF SundayFormat = FormatF(CFontF(L"Meiryo", CDirect2DWrite::Points2Dips(9)), CColorF(1.0f, 0.0f, 0.0f, 1.0f), 
//		CAlignmentF(DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
//
//};

class CCalendarControl:public CD2DWControl
{
private:
	static std::vector<std::wstring> s_weekday_texts;
private:
	std::optional<CSizeF> m_opt_cell_size;
	std::optional<CSizeF> m_opt_size;
public:
	const SolidFill& GetTodayBackground() const
	{
		static const SolidFill value(0.f, 1.f, 1.f, 0.2f); return value;
	}

	const FormatF& GetSaturdayFormat() const
	{
		static const FormatF value(CFontF(L"Meiryo", CDirect2DWrite::Points2Dips(9)), CColorF(0.0f, 0.0f, 1.0f, 1.0f),
			CAlignmentF(DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); return value;
	}
	const FormatF& GetSundayFormat() const
	{
		static const FormatF value(CFontF(L"Meiryo", CDirect2DWrite::Points2Dips(9)), CColorF(1.0f, 0.0f, 0.0f, 1.0f),
			CAlignmentF(DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); return value;
	}


public:
	reactive_property_ptr<CYearMonthDay> SelectedYearMonthDay;
	reactive_property_ptr<std::chrono::year> Year;
	reactive_property_ptr<std::chrono::month> Month;
	reactive_command_ptr<const LButtonClkEvent&> Selected;
public:
	CCalendarControl(CD2DWControl* pControl)
		:CD2DWControl(pControl){}
	virtual ~CCalendarControl() = default;
	//
	void Measure(const CSizeF& availableSize);
	CSizeF DesiredSize() const;
	void Arrange(const CRectF& rc) { m_rect = rc; }
	void Render(CDirect2DWrite* pDirect);	
	//Event
	virtual void OnPaint(const PaintEvent& e) override { Render(e.WndPtr->GetDirectPtr()); }
	virtual void OnLButtonDown(const LButtonDownEvent& e) override { *e.HandledPtr = TRUE; }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override  { *e.HandledPtr = TRUE; }
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
};