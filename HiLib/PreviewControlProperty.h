#pragma once
#include "Direct2DWrite.h"
#include "ScrollProperty.h"
#include "JsonSerializer.h"

struct PreviewControlProperty
{
public:
	SolidLine FocusedLine = SolidLine(22.f / 255.f, 160.f / 255.f, 133.f / 255.f, 1.0f, 1.0f);
	SolidFill NormalFill = SolidFill(246.f / 255.f, 246.f / 255.f, 246.f / 255.f, 1.0f);
	CRectF Padding = CRectF(2.0f, 2.0f, 2.0f, 2.0f);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(PreviewControlProperty,
		FocusedLine,
		NormalFill,
		Padding)
};

