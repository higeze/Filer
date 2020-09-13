#pragma once
#include "Direct2DWrite.h"

class IBridgeTSFInterface
{
public:
	IBridgeTSFInterface(){}
	virtual CRectF GetRectInWnd() const = 0;
};

