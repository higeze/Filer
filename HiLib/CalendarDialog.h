#pragma once
#include "D2DWDialog.h"
#include "D2DWWindow.h"

class CCalendarControl;
struct CalendarControlProperty;
class CTextBox;
struct TextBoxProperty;
class CButton;
struct ButtonProperty;

class CCalendarDialog :public CD2DWDialog
{
private:
	std::shared_ptr<CCalendarControl> m_spCalendar;
	std::shared_ptr<CTextBox> m_spYearTextBox;
	std::shared_ptr<CTextBox> m_spMonthTextBox;

	std::shared_ptr<CButton> m_spButtonPrev;
	std::shared_ptr<CButton> m_spButtonNext;
	std::shared_ptr<CButton> m_spButtonToday;
	std::shared_ptr<CButton> m_spButtonTomorrow;
	std::shared_ptr<CButton> m_spButtonBlank;

	std::shared_ptr<CButton> m_spButtonClose;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

public:
	CCalendarDialog(CD2DWControl* pParentControl);
	~CCalendarDialog();
	void Measure(const CSizeF& availableSize);
	CSizeF DesiredSize() const;
	void Arrange(const CRectF& rc);
	//std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects();

	const std::shared_ptr<CCalendarControl>& GetCalendarPtr() { return m_spCalendar; }

	//virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnRect(const RectEvent& e) override;
	virtual void OnKillFocus(const KillFocusEvent& e) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e) override 
	{ 
		CD2DWDialog::OnLButtonDown(e);
		*e.HandledPtr = TRUE;
	}
	virtual void OnLButtonUp(const LButtonUpEvent& e) override  
	{ 
		CD2DWDialog::OnLButtonUp(e);
		*e.HandledPtr = TRUE;
	}
};