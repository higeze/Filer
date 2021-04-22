#pragma once
#include "Direct2DWrite.h"

struct ScrollProperty
{
public:
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 0.4f);
	SolidFill ThumbNormalFill = SolidFill(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.4f);
	SolidFill ThumbHotFill = SolidFill(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.7f);
	SolidFill ThumbScrollFill = SolidFill(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 1.0f);

	CRectF ThumbMargin = CRectF(4.f, 4.f, 4.f, 4.f);
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

	friend void to_json(json& j, const ScrollProperty& o);
    friend void from_json(const json& j, ScrollProperty& o);
};

void to_json(json& j, const ScrollProperty& o)
{
	j = json{
		{"BackgroundFill", o.BackgroundFill},
		{"ThumbNormalFill", o.ThumbNormalFill},
		{"ThumbHotFill", o.ThumbHotFill},
		{"ThumbScrollFill", o.ThumbScrollFill},
		{"ThumbMargin", o.ThumbMargin},
		{"BandWidth", o.BandWidth },
		{"DeltaScroll", o.DeltaScroll}
	};

}

void from_json(const json& j, ScrollProperty& o)
{
	j.at("BackgroundFill").get_to(o.BackgroundFill);
	j.at("ThumbNormalFill").get_to(o.ThumbNormalFill);
	j.at("ThumbHotFill").get_to(o.ThumbHotFill);
	j.at("ThumbScrollFill").get_to(o.ThumbScrollFill);
	j.at("ThumbMargin").get_to( o.ThumbMargin);
	j.at("BandWidth").get_to(o.BandWidth);
	j.at("DeltaScroll").get_to(o.DeltaScroll);

}

