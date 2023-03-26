#pragma once
#include "ImageViewProperty.h"
#include "ScrollProperty.h"
#include "TextBoxProperty.h"
#include "StatusBarProperty.h"
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

struct ImageEditorProperty
{
public:
	std::shared_ptr<ImageViewProperty> ImageViewPropPtr = std::make_shared<ImageViewProperty>();
	std::shared_ptr<StatusBarProperty> StatusBarPropPtr = std::make_shared<StatusBarProperty>();
	std::shared_ptr<TextBoxProperty> TextBoxPropPtr = std::make_shared<TextBoxProperty>();


	friend void to_json(json& j, const ImageEditorProperty& o)
	{
		j["ImageViewPropPtr"] = o.ImageViewPropPtr;
		j["StatusBarPropPtr"] = o.StatusBarPropPtr;
		j["TextBoxPropPtr"] = o.TextBoxPropPtr;
	}

	friend void from_json(const json& j, ImageEditorProperty& o)
	{
		j.at("ImageViewPropPtr").get_to(o.ImageViewPropPtr);
		j.at("StatusBarPropPtr").get_to(o.StatusBarPropPtr);
		get_to(j, "TextBoxPropPtr", o.TextBoxPropPtr);
	}
};


