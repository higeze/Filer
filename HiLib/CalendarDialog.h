#pragma once
#include "D2DWDialog.h"

#include "D2DWWindow.h"

class CCalendarControl;
struct CalendarControlProperty;
class CTextBox;
struct TextBoxProperty;
class CButton;

class CCalendarDialog :public CD2DWDialog
{
private:
	std::shared_ptr<CCalendarControl> m_spCalendar;
	std::shared_ptr<CTextBox> m_spYearTextBox;
	std::shared_ptr<CTextBox> m_spMonthTextBox;

	std::shared_ptr<CButton> m_spButtonClose;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

public:
	CCalendarDialog(
		CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<CalendarControlProperty>& spCalendarProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp);
	~CCalendarDialog();
	void Measure(const CSizeF& availableSize);
	const CSizeF DesiredSize() const;
	void Arrange(const CRectF& rc);
	//std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects();

	const std::shared_ptr<CCalendarControl>& GetCalendarPtr() { return m_spCalendar; }

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnRect(const RectEvent& e) override;
};