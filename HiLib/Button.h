#pragma once
#include "D2DWControl.h"
#include "ReactiveProperty.h"

struct ButtonProperty;

class CButton:public CD2DWControl
{
private:
	std::shared_ptr<ButtonProperty> m_spButtonProperty;
	ReactiveCommand<void> m_command;
	ReactiveProperty<std::wstring> m_content;
	ReactiveProperty<std::wstring> m_disable_content;
public:
	CButton(CD2DWControl* pControl, const std::shared_ptr<ButtonProperty>& spButtonProperty)
		:CD2DWControl(pControl), m_spButtonProperty(spButtonProperty){}
	virtual ~CButton() = default;
	//
	ReactiveCommand<void>& GetCommand() { return m_command; }
	ReactiveProperty<std::wstring>& GetContent() { return m_content; }
	ReactiveProperty<std::wstring>& GetDisableContent() { return m_disable_content; }

	//Event
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseEnter(const MouseEnterEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;

	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;


};