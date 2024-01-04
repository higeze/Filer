#pragma once
#include "BindRow.h"
#include "Task.h"

class CTaskRow: public CBindRow<MainTask>
{
public:
	using CBindRow<MainTask>::CBindRow;

	virtual void RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc) override;
};
