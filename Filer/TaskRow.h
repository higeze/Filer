#pragma once
#include "BindRow.h"
#include "Task.h"

class CTaskRow: public CBindRow
{
public:
	using CBindRow::CBindRow;

	virtual void RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc) override;
};
