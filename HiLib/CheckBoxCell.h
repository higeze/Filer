#pragma once
#include "TextCell.h"
#include "CheckBox.h"
#include "reactive_property.h"

class CCheckBoxCell :public CCell
{
protected:
	CCheckBox m_checkBox;
public:
	CCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CCheckBoxCell() = default;

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;
	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override;
	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override;

	virtual std::wstring GetString() override;

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnChar(const CharEvent& e) override;
};