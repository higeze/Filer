#pragma once
#include "D2DWControl.h"

struct ButtonProperty
{
public:
	CRectF Padding = CRectF(2.f, 2.f, 2.f, 2.f);
	SolidFill NormalFill = SolidFill(222.f / 255.f, 222.f / 255.f, 222.f / 255.f, 1.0f);
	SolidFill HotFill = SolidFill(244.f / 255.f, 244.f / 255.f, 244.f / 255.f, 1.0f);
	SolidFill PressedFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	SolidLine BorderLine = SolidLine(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	SolidLine FocusedLine = SolidLine(22.f / 255.f, 160.f / 255, 133.f / 255.f, 1.0f, 1.0f);
	FormatF Format = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f);
	FormatF DisableFormat = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
};