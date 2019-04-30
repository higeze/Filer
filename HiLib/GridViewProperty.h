#pragma once
#include "CellProperty.h"
#include "Direct2DWrite.h"

struct ScrollProperty
{
public:
	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 0.4f);
	d2dw::SolidFill ForegroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	d2dw::CRectF ThumbMargin = d2dw::CRectF(4.f, 4.f, 4.f, 4.f);
	FLOAT BandWidth = 16.f;
	FLOAT DeltaScroll = 30.f;

	FRIEND_SERIALIZER
		template <class Archive>
	void serialize(Archive& ar)
	{
		ar("BackgroundFill", BackgroundFill);
		ar("ForegroundFill", ForegroundFill);
		ar("ThumbMargin", ThumbMargin);
		ar("BandWidth", BandWidth);
		ar("DeltaScroll", DeltaScroll);
	}

};

struct GridViewProperty
{
public:
	std::shared_ptr<BackgroundProperty> BackgroundPropPtr;
	std::shared_ptr<HeaderProperty> HeaderPropPtr; 
	std::shared_ptr<CellProperty> CellPropPtr; 
	std::shared_ptr<int> DeltaScrollPtr;
	std::shared_ptr<ScrollProperty> VScrollPropPtr;
	std::shared_ptr<ScrollProperty> HScrollPropPtr;

	GridViewProperty()
		:HeaderPropPtr(std::make_shared<HeaderProperty>()),
		BackgroundPropPtr(std::make_shared<BackgroundProperty>()),
		CellPropPtr(std::make_shared<CellProperty>()),
		VScrollPropPtr(std::make_shared<ScrollProperty>()),
		HScrollPropPtr(std::make_shared<ScrollProperty>()){ }

	virtual ~GridViewProperty() {};

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("BackgroundProperty", BackgroundPropPtr);
		ar("HeadersProperty",HeaderPropPtr);
		ar("CellsProperty",CellPropPtr);
		ar("VScrollProperty", VScrollPropPtr);
		ar("HScrollProperty", HScrollPropPtr);
    }
};

