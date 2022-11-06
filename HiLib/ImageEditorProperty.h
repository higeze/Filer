#pragma once
#include "ImageViewProperty.h"
#include "ScrollProperty.h"
#include "StatusBarProperty.h"
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

struct ImageEditorProperty
{
public:
	std::shared_ptr<ImageViewProperty> ImageViewPropPtr = std::make_shared<ImageViewProperty>();
	std::shared_ptr<StatusBarProperty> StatusBarPropPtr = std::make_shared<StatusBarProperty>();


	friend void to_json(json& j, const ImageEditorProperty& o)
	{
		j["ImageViewPropPtr"] = o.ImageViewPropPtr;
		j["StatusBarPropPtr"] = o.StatusBarPropPtr;
	}

	friend void from_json(const json& j, ImageEditorProperty& o)
	{
		j.at("ImageViewPropPtr").get_to(o.ImageViewPropPtr);
		j.at("StatusBarPropPtr").get_to(o.StatusBarPropPtr);
	}
};


