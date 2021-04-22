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
    friend void to_json(json& j, const ProgressProperty& o);
    friend void from_json(const json& j, ProgressProperty& o);
};

void to_json(json& j, const ProgressProperty& o)
{
	j = json{
		{"Border", o.Border},
		{"BackgroundFill", o.BackgroundFill},
		{"ForegroundFill", o.ForegroundFill}
	};
}
void from_json(const json& j, ProgressProperty& o)
{
	j.at("Border").get_to(o.Border);
	j.at("BackgroundFill").get_to(o.BackgroundFill);
	j.at("ForegroundFill").get_to(o.ForegroundFill);
}


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
    friend void to_json(json& j, const GridViewProperty& o);
    friend void from_json(const json& j, GridViewProperty& o);
};

void to_json(json& j, const GridViewProperty& o)
{
	to_json(j, static_cast<const SheetProperty&>(o));
	j["VScrollProperty"] = o.VScrollPropPtr;
	j["HScrollProperty"] = o.HScrollPropPtr;
}
void from_json(const json& j, GridViewProperty& o)
{
	from_json(j, static_cast<SheetProperty&>(o));
	j.at("VScrollProperty").get_to(o.VScrollPropPtr);
	j.at("HScrollProperty").get_to(o.HScrollPropPtr);
}

