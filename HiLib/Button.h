#pragma once
#include "D2DWControl.h"
#include "reactive_property.h"
#include "reactive_command.h"


struct ButtonProperty;

class CButton:public CD2DWControl
{
private:
	std::shared_ptr<ButtonProperty> m_spButtonProperty;
	std::optional<CSizeF> m_opt_size;
public:
	reactive_command_ptr<void> Command;
	reactive_property_ptr<std::wstring> Content;
	reactive_property_ptr<std::wstring> DisableContent;
public:
	CButton(CD2DWControl* pControl, const std::shared_ptr<ButtonProperty>& spButtonProperty)
		:CD2DWControl(pControl), m_spButtonProperty(spButtonProperty){}
	virtual ~CButton() = default;

	void Measure(const CSizeF& availableSize);
	CSizeF DesiredSize() const;
	void Arrange(const CRectF& rc) { m_rect = rc; }

	//Event
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseEnter(const MouseEnterEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;

	virtual void OnKeyDown(const KeyDownEvent& e) override;


};