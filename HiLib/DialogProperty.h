#pragma once
#include "Direct2DWrite.h"
#include "JsonSerializer.h"

struct DialogProperty
{
	FormatF TitleFormat = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f);
	SolidLine Line = SolidLine(30.f / 255.f, 30.f / 255.f, 30.f / 255.f, 1.0f, 1.0f);
	SolidFill BackgroundFill = SolidFill(246.f / 255.f, 246.f / 255.f, 246.f / 255.f, 1.0f);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(
		DialogProperty,
		TitleFormat,
		Line,
		BackgroundFill)
};