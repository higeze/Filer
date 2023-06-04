#pragma once
#include "BindRow.h"
#include "Task.h"

class CTaskRow: public CBindRow<MainTask>
{
public:
	using CBindRow<MainTask>::CBindRow;

	virtual void OnPaint(const PaintEvent& e) override;
};
