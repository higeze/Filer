#pragma once
#include "D2DWControl.h"
#include "TextBlockProperty.h"
#include "reactive_string.h"

class CTextBlock:public CD2DWControl
{
public:
	reactive_wstring_ptr Text;
public:
	CTextBlock(CD2DWControl* pControl)
		:CD2DWControl(pControl), Text(){}
	virtual ~CTextBlock() = default;

	CSizeF MeasureOverride(const CSizeF& availableSize) override;
	CSizeF MeasureSize(const std::wstring& text);
	//Event
	virtual void OnPaint(const PaintEvent& e) override;
};