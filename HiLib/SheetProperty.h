#pragma once
#include "CellProperty.h"

struct SheetProperty
{
public:
	std::shared_ptr<BackgroundProperty> BackgroundPropPtr;
	std::shared_ptr<HeaderProperty> HeaderPropPtr;
	std::shared_ptr<CellProperty> CellPropPtr;

	SheetProperty()
		: BackgroundPropPtr(std::make_shared<BackgroundProperty>()),
		HeaderPropPtr(std::make_shared<HeaderProperty>()),
		CellPropPtr(std::make_shared<CellProperty>())
	{}
	virtual ~SheetProperty() = default;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("BackgroundProperty", BackgroundPropPtr);
		ar("HeadersProperty", HeaderPropPtr);
		ar("CellsProperty", CellPropPtr);
	}
    friend void to_json(json& j, const SheetProperty& o);
    friend void from_json(const json& j, SheetProperty& o);
};

void to_json(json& j, const SheetProperty& o)
{
	j = json{
		{"BackgroundProperty", o.BackgroundPropPtr },
		{"HeadersProperty", o.HeaderPropPtr},
		{"CellsProperty", o.CellPropPtr}
	};

}
void from_json(const json& j, SheetProperty& o)
{
	j.at("BackgroundProperty").get_to(o.BackgroundPropPtr);
	j.at("HeadersProperty").get_to(o.HeaderPropPtr);
	j.at("CellsProperty").get_to(o.CellPropPtr);
}
