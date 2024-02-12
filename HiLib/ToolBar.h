#pragma once

#include "D2DWControl.h"

class CToolBar: public CD2DWControl
{
public:
	CToolBar(CD2DWControl* pParentControl = nullptr);
	void Measure(const CSizeF& availableSize) override;
	void Arrange(const CRectF& rc) override;

	void OnPaint(const PaintEvent& e) override;

};