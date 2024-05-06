#pragma once
#include "TextBoxProperty.h"
#include "ScrollProperty.h"
#include "StatusBarProperty.h"
#include "reactive_property.h"
#include "reactive_vector.h"
#include "JsonSerializer.h"

//struct SyntaxAppearance
//{
//	std::wstring Regex;
//	//std::regex Re;
//	SyntaxFormatF SyntaxFormat;
//	SyntaxAppearance()
//		:Regex(), SyntaxFormat(){}
//	SyntaxAppearance(const std::wstring& regex, const SyntaxFormatF& syntaxformat)
//		:Regex(regex), SyntaxFormat(syntaxformat){}
//
//	auto operator<=>(const SyntaxAppearance&) const = default;
//
//	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SyntaxAppearance,
//		Regex,
//		SyntaxFormat)
//};
//
//struct ExecutableAppearance
//{
//	std::wstring Regex;
//	SyntaxFormatF SyntaxFormat;
//	ExecutableAppearance()
//		:Regex(), SyntaxFormat(){}
//	ExecutableAppearance(const std::wstring& regex, const SyntaxFormatF& syntaxformat)
//		:Regex(regex), SyntaxFormat(syntaxformat){}
//
//	auto operator<=>(const ExecutableAppearance&) const = default;
//
//	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExecutableAppearance,
//		Regex,
//		SyntaxFormat)
//};
//
//struct EditorScrollProperty : public ScrollProperty
//{
//public:
//	SolidFill FindHighliteFill = SolidFill(244.f / 255, 167.f / 255, 33.f / 255, 1.f);
//
//	friend void to_json(json& j, const EditorScrollProperty& o)
//	{
//		to_json(j, static_cast<const ScrollProperty&>(o));
//		j["FindHighliteFill"] = o.FindHighliteFill;
//	}
//	friend void from_json(const json& j, EditorScrollProperty& o)
//	{
//		from_json(j, static_cast<ScrollProperty&>(o));
//		j.at("FindHighliteFill").get_to(o.FindHighliteFill);
//	}
//};

//

//struct EditorProperty
//{
//public:
//	//std::shared_ptr<EditorTextBoxProperty> EditorTextBoxPropPtr = std::make_shared<EditorTextBoxProperty>();
//	//std::shared_ptr<StatusBarProperty> StatusBarPropPtr = std::make_shared<StatusBarProperty>();
//
//	friend void to_json(json& j, const EditorProperty& o)
//	{
//		j["EditorTextBoxPropPtr"] = o.EditorTextBoxPropPtr;
//		j["StatusBarPropPtr"] = o.StatusBarPropPtr;
//	}
//
//	friend void from_json(const json& j, EditorProperty& o)
//	{
//		get_to(j, "EditorTextBoxPropPtr", o.EditorTextBoxPropPtr);
//		get_to(j, "StatusBarPropPtr", o.StatusBarPropPtr);
//	}
//};
//
