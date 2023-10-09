#pragma once
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "D2DWWindow.h"
#include "D2DWTypes.h"
#include <chrono>
#include "reactive_property.h"
#include "reactive_command.h"

struct CalendarControlProperty
{
	CRectF Padding = CRectF(2,2,2,2);
	FormatF Format = FormatF(CFontF(L"Meiryo", 7.0f), CColorF(0.0f, 0.0f, 0.0f, 1.0f), CAlignmentF(DWRITE_TEXT_ALIGNMENT_TRAILING));

};

class CCalendarControl:public CD2DWControl
{
private:
	std::optional<CSizeF> m_opt_cell_size;
	std::optional<CSizeF> m_opt_size;
	std::shared_ptr<CalendarControlProperty> m_spProp;
public:
	reactive_command_ptr<void> SelectedCommand;
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
	virtual void OnLButtonClk(const LButtonClkEvent& e) override {}
};