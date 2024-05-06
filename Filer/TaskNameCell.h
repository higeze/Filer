#pragma once
#include "BindTextCell.h"
#include "Task.h"

class CTaskNameCell : public CBindTextCell<MainTask>
{
public:
	using CBindTextCell::CBindTextCell;

	void PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rcPaint) override;
};
