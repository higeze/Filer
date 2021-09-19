#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"

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

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Border", Border);
		ar("BackgroundFill", BackgroundFill);
		ar("ForegroundFill", ForegroundFill);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(StatusBarProperty,
		Format,
		Border,
		BackgroundFill,
		ForegroundFill)
};