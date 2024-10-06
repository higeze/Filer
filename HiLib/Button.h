#pragma once
#include "D2DWControl.h"
#include "reactive_property.h"
#include "reactive_command.h"


class CButton:public CD2DWControl
{
private:
	//std::optional<CSizeF> m_opt_size;
public:
	const SolidFill& GetNormalBackground() const override
	{
		static const SolidFill value(222.f / 255.f, 222.f / 255.f, 222.f / 255.f, 1.0f); return value;
	}
	virtual const SolidFill& GetPressedBackground() const
	{
		static const SolidFill value(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f); return value;
	}
	virtual const FormatF& GetDisableFormat() const
	{
		static const FormatF value(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f); return value;
	}


public:
	reactive_command_ptr<void> Command;
	reactive_property_ptr<std::wstring> Content;
	reactive_property_ptr<std::wstring> DisableContent;
public:
	CButton(CD2DWControl* pControl = nullptr)
		:CD2DWControl(pControl){}
	virtual ~CButton() = default;

	//MeasureArrrange
	CSizeF MeasureOverride(const CSizeF& availableSize) override;

	//Event
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseEnter(const MouseEnterEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;

	virtual void OnKeyDown(const KeyDownEvent& e) override;


};