#pragma once
#include "BindCheckBoxCell.h"
#include "Task.h"

class CTaskCheckBoxCell : public CBindCheckBoxCell<MainTask>
{
public:
	using CBindCheckBoxCell::CBindCheckBoxCell;

	void PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rcPaint) override;
};
