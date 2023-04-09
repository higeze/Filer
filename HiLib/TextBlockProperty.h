#pragma once
#include "Direct2DWrite.h"
#include "JsonSerializer.h"

struct TextBlockProperty
{
public:
	FormatF Format = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f);
	FormatF DisableFormat = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	CRectF Padding = CRectF(2.f, 2.f, 2.f, 2.f);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextBlockProperty,
		Format,
		DisableFormat,
		Padding)
};
