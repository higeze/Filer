#pragma once
#include "TextBoxProperty.h"
#include "StatusBarProperty.h"
#include "ReactiveProperty.h"

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

	template <class Archive>
	void save(Archive& ar)
	{
		ar("Regex", Regex);
		ar("SyntaxFormat", SyntaxFormat);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("Regex", Regex);
		ar("SyntaxFormat", SyntaxFormat);
	}

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

	template <class Archive>
	void save(Archive& ar)
	{
		ar("Regex", Regex);
		ar("SyntaxFormat", SyntaxFormat);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("Regex", Regex);
		ar("SyntaxFormat", SyntaxFormat);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExecutableAppearance,
		Regex,
		SyntaxFormat)
};

struct TextEditorProperty :public TextBoxProperty
{
	ReactiveVectorProperty<std::tuple<SyntaxAppearance>> SyntaxAppearances;
	std::vector<ExecutableAppearance> ExecutableAppearances;
	std::shared_ptr<StatusBarProperty> StatusBarPropPtr;

	TextEditorProperty():TextBoxProperty(){}

	template <class Archive>
	void save(Archive& ar)
	{
		TextBoxProperty::save(ar);
		ar("SyntaxAppearances", SyntaxAppearances);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		TextBoxProperty::load(ar);
		ar("SyntaxAppearances", SyntaxAppearances);

		if (SyntaxAppearances.empty()) {
			SyntaxAppearances.push_back(
				std::make_tuple(
				SyntaxAppearance(L"/\\*.*?\\*/",
				SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false, false))));
			SyntaxAppearances.push_back(
				std::make_tuple(
				SyntaxAppearance(L"//.*?\n",
				SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false, false))));

		}
	}

	friend void to_json(json& j, const TextEditorProperty& o)
	{
		to_json(j, static_cast<const TextBoxProperty&>(o));
		j["SyntaxAppearances"] = o.SyntaxAppearances;
		j["ExecutableAppearance"] = o.ExecutableAppearances;
		j["StatusBarProperty"] = o.StatusBarPropPtr;
	}

	friend void from_json(const json& j, TextEditorProperty& o)
	{
		from_json(j, static_cast<TextBoxProperty&>(o));
		j.at("SyntaxAppearances").get_to(o.SyntaxAppearances);
		j.at("ExecutableAppearance").get_to(o.ExecutableAppearances);
		get_to_nothrow(j, "StatusBarProperty", o.StatusBarPropPtr);

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

		if (!o.StatusBarPropPtr) {
			o.StatusBarPropPtr = std::make_shared<StatusBarProperty>();
		}
	}

};

