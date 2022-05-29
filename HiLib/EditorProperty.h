#pragma once
#include "TextBoxProperty.h"
#include "ScrollProperty.h"
#include "StatusBarProperty.h"
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

struct SyntaxAppearance
{
	std::wstring Regex;
	//std::regex Re;
	SyntaxFormatF SyntaxFormat;
	SyntaxAppearance()
		:Regex(), SyntaxFormat(){}
	SyntaxAppearance(const std::wstring& regex, const SyntaxFormatF& syntaxformat)
		:Regex(regex), SyntaxFormat(syntaxformat){}

	auto operator<=>(const SyntaxAppearance&) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SyntaxAppearance,
		Regex,
		SyntaxFormat)
};

struct ExecutableAppearance
{
	std::wstring Regex;
	SyntaxFormatF SyntaxFormat;
	ExecutableAppearance()
		:Regex(), SyntaxFormat(){}
	ExecutableAppearance(const std::wstring& regex, const SyntaxFormatF& syntaxformat)
		:Regex(regex), SyntaxFormat(syntaxformat){}

	auto operator<=>(const ExecutableAppearance&) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExecutableAppearance,
		Regex,
		SyntaxFormat)
};

struct EditorScrollProperty : public ScrollProperty
{
public:
	SolidFill FindHighliteFill = SolidFill(244.f / 255, 167.f / 255, 33.f / 255, 1.f);

	friend void to_json(json& j, const EditorScrollProperty& o)
	{
		to_json(j, static_cast<const ScrollProperty&>(o));
		j["FindHighliteFill"] = o.FindHighliteFill;
	}
	friend void from_json(const json& j, EditorScrollProperty& o)
	{
		from_json(j, static_cast<ScrollProperty&>(o));
		j.at("FindHighliteFill").get_to(o.FindHighliteFill);
	}
};

struct EditorTextBoxProperty :public TextBoxProperty
{
	ReactiveVectorProperty<std::tuple<SyntaxAppearance>> SyntaxAppearances;
	std::vector<ExecutableAppearance> ExecutableAppearances;
	SolidFill FindHighliteFill = SolidFill(244.f / 255, 167.f / 255, 33.f / 255, 100.f / 255);

	EditorTextBoxProperty():
		TextBoxProperty(std::make_shared<EditorScrollProperty>(), std::make_shared<EditorScrollProperty>())
	
	{
		auto a = 9;
	}

	virtual ~EditorTextBoxProperty() = default;

	friend void to_json(json& j, const EditorTextBoxProperty& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(ScrollProperty, EditorScrollProperty);

		to_json(j, static_cast<const TextBoxProperty&>(o));
		j["VScrollPropPtr"] = std::static_pointer_cast<EditorScrollProperty>(o.VScrollPropPtr);
		j["SyntaxAppearances"] = o.SyntaxAppearances;
		j["ExecutableAppearance"] = o.ExecutableAppearances;
		j["FindHighliteFill"] = o.FindHighliteFill;
	}

	friend void from_json(const json& j, EditorTextBoxProperty& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(ScrollProperty, EditorScrollProperty);

		from_json(j, static_cast<TextBoxProperty&>(o));
		get_to(j, "VScrollPropPtr", o.VScrollPropPtr);
		get_to(j, "SyntaxAppearances", o.SyntaxAppearances);
		get_to(j, "FindHighliteFill", o.FindHighliteFill);
		get_to(j, "ExecutableAppearance", o.ExecutableAppearances);

		if (o.SyntaxAppearances.empty()) {
			o.SyntaxAppearances.push_back(
				std::make_tuple(
					SyntaxAppearance(L"/\\*.*?\\*/",
						SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false, false))));
			o.SyntaxAppearances.push_back(
				std::make_tuple(
					SyntaxAppearance(L"//.*?\n",
						SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false, false))));
		}
		
		if (o.ExecutableAppearances.empty()) {
			o.ExecutableAppearances.push_back(
				ExecutableAppearance{
				LR"((?:[a-zA-Z]:|\\\\[a-zA-Z0-9_.$Åú-]+\\[a-zA-Z0-9_.$Åú-]+)\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]*)",
				SyntaxFormatF(CColorF(0.0f, 0.0f, 1.0f, 1.0f), false, true)
				});
			o.ExecutableAppearances.push_back(
				ExecutableAppearance{
				LR"(https?://[\w!?/+\-_~;.,*&@#$%()'[\]=]+)",
				SyntaxFormatF(CColorF(0.0f, 0.0f, 1.0f, 1.0f), false, true)
				});
		}
	}

};

struct EditorProperty
{
public:
	std::shared_ptr<EditorTextBoxProperty> EditorTextBoxPropPtr = std::make_shared<EditorTextBoxProperty>();
	std::shared_ptr<StatusBarProperty> StatusBarPropPtr = std::make_shared<StatusBarProperty>();

	friend void to_json(json& j, const EditorProperty& o)
	{
		j["EditorTextBoxPropPtr"] = o.EditorTextBoxPropPtr;
		j["StatusBarPropPtr"] = o.StatusBarPropPtr;
	}

	friend void from_json(const json& j, EditorProperty& o)
	{
		get_to(j, "EditorTextBoxPropPtr", o.EditorTextBoxPropPtr);
		get_to(j, "StatusBarPropPtr", o.StatusBarPropPtr);
	}
};

