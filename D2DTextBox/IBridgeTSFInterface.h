#pragma once
#include "Direct2DWrite.h"

class IBridgeTSFInterface
{
public:
	IBridgeTSFInterface(){}
	virtual d2dw::CRectF GetRectInWnd() const = 0;
};

