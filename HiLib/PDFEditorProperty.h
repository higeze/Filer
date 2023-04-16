#pragma once
#include "PDFViewProperty.h"
#include "TextBoxProperty.h"
#include "TextBlockProperty.h"
#include "ScrollProperty.h"
#include "StatusBarProperty.h"
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

struct PDFEditorProperty
{
public:
	std::shared_ptr<PdfViewProperty> PDFViewPropPtr = std::make_shared<PdfViewProperty>();
	std::shared_ptr<TextBoxProperty> TextBoxPropPtr = std::make_shared<TextBoxProperty>();
	std::shared_ptr<TextBlockProperty> TextBlockPropPtr = std::make_shared<TextBlockProperty>();
	std::shared_ptr<StatusBarProperty> StatusBarPropPtr = std::make_shared<StatusBarProperty>();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(PDFEditorProperty,
		PDFViewPropPtr,
		TextBoxPropPtr,
		TextBlockPropPtr,
		StatusBarPropPtr)

	//friend void to_json(json& j, const PDFEditorProperty& o)
	//{
	//	j["PdfViewPropPtr"] = o.PDFViewPropPtr;
	//	j["StatusBarPropPtr"] = o.StatusBarPropPtr;
	//}

	//friend void from_json(const json& j, PDFEditorProperty& o)
	//{
	//	j.at("PdfViewPropPtr").get_to(o.PDFViewPropPtr);
	//	j.at("StatusBarPropPtr").get_to(o.StatusBarPropPtr);
	//}
};

