#pragma once
#include "ImageViewProperty.h"
#include "ScrollProperty.h"
#include "TextBoxProperty.h"
#include "TextBlockProperty.h"
#include "StatusBarProperty.h"
#include "JsonSerializer.h"

//struct ImageEditorProperty
//{
//public:
//	std::shared_ptr<TextBlockProperty> TextBlockPropPtr = std::make_shared<TextBlockProperty>();
//
//
//	//friend void to_json(json& j, const ImageEditorProperty& o)
//	//{
//	//	j["ImageViewPropPtr"] = o.ImageViewPropPtr;
//	//	j["StatusBarPropPtr"] = o.StatusBarPropPtr;
//	//	j["TextBoxPropPtr"] = o.TextBoxPropPtr;
//	//	j["TextBlockPropPtr"] = o.TextBlockPropPtr;
//	//}
//
//	//friend void from_json(const json& j, ImageEditorProperty& o)
//	//{
//	//	j.at("ImageViewPropPtr").get_to(o.ImageViewPropPtr);
//	//	j.at("StatusBarPropPtr").get_to(o.StatusBarPropPtr);
//	//	get_to(j, "TextBoxPropPtr", o.TextBoxPropPtr);
//	//	get_to(j, "TextBlockPropPtr", o.TextBlockPropPtr);
//	//}
//};


