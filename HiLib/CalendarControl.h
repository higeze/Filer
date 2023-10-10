#pragma once
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "D2DWWindow.h"
#include "D2DWTypes.h"
#include "YearMonthDay.h"
#include <chrono>
#include "reactive_property.h"
#include "reactive_command.h"

struct CalendarControlProperty
{
	CRectF Padding = CRectF(2,2,2,2);
	SolidFill TodayFill = SolidFill(0.f, 0.f, 1.f, 0.3f);
	SolidLine SelectedLine = SolidLine(22.f / 255.f, 160.f / 255.f, 133.f / 255.f, 1.0f, 1.0f);
	FormatF Format = FormatF(CFontF(L"Meiryo", CDirect2DWrite::Points2Dips(9)), CColorF(0.0f, 0.0f, 0.0f, 1.0f), 
		CAlignmentF(DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

};

class CCalendarControl:public CD2DWControl
{
private:
	std::optional<CSizeF> m_opt_cell_size;
	std::optional<CSizeF> m_opt_size;
	std::shared_ptr<CalendarControlProperty> m_spProp;
public:
	reactive_property_ptr<CYearMonthDay> SelectedYearMonthDay;
	reactive_property_ptr<std::chrono::year> Year;
	reactive_property_ptr<std::chrono::month> Month;
public:
	CCalendarControl(CD2DWControl* pControl, const std::shared_ptr<CalendarControlProperty>& spProp)
		:CD2DWControl(pControl), m_spProp(spProp){}
	virtual ~CCalendarControl() = default;
	//
	void Measure(const CSizeF& availableSize);
	const CSizeF& DesiredSize() const;
	void Arrange(const CRectF& rc) { m_rect = rc; }
	void Render(CDirect2DWrite* pDirect);	
	//Event
	virtual void OnPaint(const PaintEvent& e) override { Render(e.WndPtr->GetDirectPtr()); }
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
};