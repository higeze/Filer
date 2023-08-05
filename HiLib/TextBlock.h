#pragma once
#include "D2DWControl.h"
#include "TextBlockProperty.h"
#include "reactive_string.h"

class CTextBlock:public CD2DWControl
{
private:
	std::shared_ptr<TextBlockProperty> m_spProperty;
public:
	reactive_wstring_ptr Text;
public:
	CTextBlock(CD2DWControl* pControl, const std::shared_ptr<TextBlockProperty>& spProperty)
		:CD2DWControl(pControl), m_spProperty(spProperty){}
	virtual ~CTextBlock() = default;

	CSizeF MeasureSize();
	CSizeF MeasureSize(const std::wstring& text);
	//Event
	virtual void OnPaint(const PaintEvent& e) override;
};