#pragma once
#include "D2DWControl.h"
#include "ReactiveProperty.h"

struct ButtonProperty
{
public:
	SolidFill NormalFill = SolidFill(222.f / 255.f, 222.f / 255.f, 222.f / 255.f, 1.0f);
	SolidFill HotFill = SolidFill(244.f / 255.f, 244.f / 255.f, 244.f / 255.f, 1.0f);
	SolidFill PressedFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	SolidLine BorderLine = SolidLine(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	SolidLine FocusedLine = SolidLine(22.f / 255.f, 160.f / 255, 133.f / 255.f, 1.0f, 1.0f);
	FormatF Format = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f);
	FormatF DisableFormat = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("NormalFill", NormalFill);
		ar("HotFillFill", NormalFill);
		ar("PressedFill", NormalFill);
		ar("BorderLine", BorderLine);
		ar("Format", Format);
	}
};


class CButton:public CD2DWControl
{
private:
	std::shared_ptr<ButtonProperty> m_spButtonProperty;
	ReactiveCommand<void> m_command;
	ReactiveProperty<std::wstring> m_content;
public:
	CButton(CD2DWControl* pControl, const std::shared_ptr<ButtonProperty>& spButtonProperty)
		:CD2DWControl(pControl), m_spButtonProperty(spButtonProperty){}
	virtual ~CButton() = default;
	//
	ReactiveCommand<void>& GetCommand() { return m_command; }
	ReactiveProperty<std::wstring>& GetContent() { return m_content; }

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