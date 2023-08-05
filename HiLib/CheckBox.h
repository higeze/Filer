#pragma once
#include "Direct2DWrite.h"
#include "CheckBoxType.h"
#include "CheckBoxState.h"
#include "reactive_property.h"

class CCheckBox
{
private:
	CRectF m_rect;
	CSizeF m_size;
public:
	reactive_property_ptr<CheckBoxType> Type;
	reactive_property_ptr<CheckBoxState> State;

	CCheckBox(CheckBoxType&& type, CheckBoxState&& state);
	void Measure(const CSizeF& availableSize) { m_size = availableSize; }
	const CSizeF& DesiredSize() const { return m_size; }
	void Arrange(const CRectF& rc) { m_rect = rc; }
	void Render(CDirect2DWrite* pDirect);
	void Toggle();
	const void* HitTestCore(const CPointF&) const;//TODO

};
