#pragma once
#include "Direct2DWrite.h"

struct ScrollProperty
{
public:
	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 0.4f);
	d2dw::SolidFill ThumbNormalFill = d2dw::SolidFill(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.4f);
	d2dw::SolidFill ThumbHotFill = d2dw::SolidFill(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.7f);
	d2dw::SolidFill ThumbScrollFill = d2dw::SolidFill(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 1.0f);

	d2dw::CRectF ThumbMargin = d2dw::CRectF(4.f, 4.f, 4.f, 4.f);
	FLOAT BandWidth = 16.f;
	FLOAT DeltaScroll = 30.f;

	FRIEND_SERIALIZER
		template <class Archive>
	void serialize(Archive& ar)
	{
		ar("BackgroundFill", BackgroundFill);
		ar("ThumbNormalFill", ThumbNormalFill);
		ar("ThumbHotFill", ThumbHotFill);
		ar("ThumbScrollFill", ThumbScrollFill);
		ar("ThumbMargin", ThumbMargin);
		ar("BandWidth", BandWidth);
		ar("DeltaScroll", DeltaScroll);
	}
};

