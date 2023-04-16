#pragma once
#include "D2DWControl.h"
#include "TextBlockProperty.h"
#include "ReactiveProperty.h"

class CTextBlock:public CD2DWControl
{
private:
	std::shared_ptr<TextBlockProperty> m_spProperty;
	ReactiveWStringProperty m_text;
public:
	CTextBlock(CD2DWControl* pControl, const std::shared_ptr<TextBlockProperty>& spProperty)
		:CD2DWControl(pControl), m_spProperty(spProperty){}
	virtual ~CTextBlock() = default;
	//
	ReactiveWStringProperty& GetText() { return m_text; }

	CSizeF MeasureSize();
	CSizeF MeasureSize(const std::wstring& text);
	//Event
	virtual void OnPaint(const PaintEvent& e) override;
};