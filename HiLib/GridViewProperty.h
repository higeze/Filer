#pragma once
#include "Direct2DWrite.h"
#include "CellProperty.h"
#include "SheetProperty.h"
#include "ScrollProperty.h"

struct ProgressProperty
{
public:
	d2dw::FormatF Format = d2dw::FormatF();

	d2dw::SolidLine Border = d2dw::SolidLine(230.f / 255.f, 230.f / 255.f, 230.f / 255.f, 1.0f);
	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	d2dw::SolidFill ForegroundFill = d2dw::SolidFill(0.f / 255.f, 200.f / 255.f, 0.f / 255.f, 1.0f);

	FRIEND_SERIALIZER
		template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Border", Border);
		ar("BackgroundFill", BackgroundFill);
		ar("ForegroundFill", ForegroundFill);
	}
};

struct GridViewProperty:public SheetProperty
{
public:
	std::shared_ptr<ScrollProperty> VScrollPropPtr;
	std::shared_ptr<ScrollProperty> HScrollPropPtr;

	GridViewProperty()
		:SheetProperty(),
		VScrollPropPtr(std::make_shared<ScrollProperty>()),
		HScrollPropPtr(std::make_shared<ScrollProperty>()){ }

	virtual ~GridViewProperty() = default;

    template <class Archive>
    void serialize(Archive& ar)
    {
		SheetProperty::serialize(ar);
		ar("VScrollProperty", VScrollPropPtr);
		ar("HScrollProperty", HScrollPropPtr);
    }
};

