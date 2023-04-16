#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include "JsonSerializer.h"

/*********************/
/* StatusBarProperty */
/*********************/
struct StatusBarProperty
{
public:
	FormatF Format = FormatF();
	SolidLine Border = SolidLine(230.f / 255.f, 230.f / 255.f, 230.f / 255.f, 1.0f);
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	SolidFill ForegroundFill = SolidFill(0.f, 0.f, 0.f, 1.0f);
	CRectF Padding = CRectF(1.f, 1.f, 1.f, 1.f);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(StatusBarProperty,
		Format,
		Border,
		BackgroundFill,
		ForegroundFill,
		Padding)
};