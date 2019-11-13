#pragma once
#include "Direct2DWrite.h"

class IBridgeTSFInterface
{
public:
	IBridgeTSFInterface(){}
	virtual d2dw::CRectF GetClientRect() const = 0;
	virtual d2dw::CRectF GetContentRect() const = 0;
//	virtual IDWriteTextFormat* GetFormat() = 0;
//	virtual V4::FRectFBoxModel GetClientRectEx() = 0;
};

