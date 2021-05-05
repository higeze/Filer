#pragma once
#include "Direct2DWrite.h"
#include "CellProperty.h"
#include "SheetProperty.h"
#include "ScrollProperty.h"

struct ProgressProperty
{
public:
	FormatF Format = FormatF();

	SolidLine Border = SolidLine(230.f / 255.f, 230.f / 255.f, 230.f / 255.f, 1.0f);
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	SolidFill ForegroundFill = SolidFill(0.f / 255.f, 200.f / 255.f, 0.f / 255.f, 1.0f);

	FRIEND_SERIALIZER
		template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Border", Border);
		ar("BackgroundFill", BackgroundFill);
		ar("ForegroundFill", ForegroundFill);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ProgressProperty,
		Border,
		BackgroundFill,
		ForegroundFill)
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

	friend void to_json(json& j, const GridViewProperty& o)
	{
		to_json(j, static_cast<const SheetProperty&>(o));
		j["VScrollProperty"] = o.VScrollPropPtr;
		j["HScrollProperty"] = o.HScrollPropPtr;
	}
	friend void from_json(const json& j, GridViewProperty& o)
	{
		from_json(j, static_cast<SheetProperty&>(o));
		j.at("VScrollProperty").get_to(o.VScrollPropPtr);
		j.at("HScrollProperty").get_to(o.HScrollPropPtr);
	}
};