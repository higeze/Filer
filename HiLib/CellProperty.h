#pragma once
#include "Direct2DWrite.h"
#include "ScrollProperty.h"
#include <regex>
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

enum class PicturePositon
{
	Fill,
	Fit,
	Stretch,
	Tile,
	Center
};

struct BackgroundProperty
{
public:
	std::shared_ptr<SolidFill> m_brush;
	std::shared_ptr<bool> m_usePicture;
	std::shared_ptr<std::wstring> m_picturePath;
	std::shared_ptr<PicturePositon> m_picturePosition;

	BackgroundProperty()
		:m_brush(std::make_shared<SolidFill>(246.0f/255, 246.0f/255, 246.0f/255, 1.0f)),//asbestos
		m_usePicture(std::make_shared<bool>(false)),
		m_picturePath(std::make_shared<std::wstring>()),
		m_picturePosition(std::make_shared<PicturePositon>(PicturePositon::Center)){}
	virtual ~BackgroundProperty() = default;

    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("BackgroundBrush",m_brush);
		ar("UsePicture",m_usePicture);
		ar("PicturePath",m_picturePath);
		ar("PicturePosition",m_picturePosition);
    }

	friend void to_json(json& j, const BackgroundProperty& o);
    friend void from_json(const json& j, BackgroundProperty& o);
};
void to_json(json& j, const BackgroundProperty& o)
{
	j = json{
		{"BackgroundBrush", o.m_brush},
		{"UsePicture", o.m_usePicture},
		{"PicturePath", o.m_picturePath},
		{"PicturePosition", o.m_picturePosition}
	};
}
void from_json(const json& j, BackgroundProperty& o)
{
	j.at("BackgroundBrush").get_to(o.m_brush);
	j.at("UsePicture").get_to(o.m_usePicture);
	j.at("PicturePath").get_to(o.m_picturePath);
	j.at("PicturePosition").get_to(o.m_picturePosition);
}

struct TextboxProperty
{
public:
	std::shared_ptr<FormatF> Format;
	std::shared_ptr<SolidLine> Line;
	std::shared_ptr<SolidLine> EditLine;
	std::shared_ptr<SolidLine> FocusedLine;
	std::shared_ptr<SolidLine> BlankLine;
	std::shared_ptr<SolidFill> NormalFill;
	std::shared_ptr<SolidFill> FocusedFill;
	std::shared_ptr<SolidFill> SelectedFill;
	std::shared_ptr<SolidFill> UnfocusSelectedFill;
	std::shared_ptr<SolidFill> HotFill;
	std::shared_ptr<CRectF> Padding;
	std::shared_ptr<ScrollProperty> VScrollPropPtr;
	std::shared_ptr<ScrollProperty> HScrollPropPtr;
	bool IsWrap = true;


public:
	TextboxProperty()
		:Format(std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
		Line(std::make_shared<SolidLine>(221.f/255.f, 206.f/255.f, 188.f/255.f, 1.0f, 1.0f)),
		EditLine(std::make_shared<SolidLine>(0.0f/255.f, 0.0f/255.f, 255.f/255.f, 1.0f, 1.0f)),
		FocusedLine(std::make_shared<SolidLine>(22.f/255.f, 160.f/255.f, 133.f/255.f, 1.0f, 1.0f)),
		BlankLine(std::make_shared<SolidLine>(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.6f, 1.0f)),
		NormalFill(std::make_shared<SolidFill>(246.f/255.f, 246.f/255.f, 246.f/255.f, 1.0f)),
		FocusedFill(std::make_shared<SolidFill>(0.0f/255.f, 224.f/255.f, 226.f/255.f, 1.0f)),
		SelectedFill(std::make_shared<SolidFill>(150.f/255.f, 200.f/255.f, 240.f/255.f, 0.5f)),
		UnfocusSelectedFill(std::make_shared<SolidFill>(224.f/255.f, 224.f/255.f, 224.f/255.f, 0.5f)),
		HotFill(std::make_shared<SolidFill>(1.0f, 1.0f, 1.0f, 0.3f)),
		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)),
		VScrollPropPtr(std::make_shared<ScrollProperty>()),
		HScrollPropPtr(std::make_shared<ScrollProperty>()),
		IsWrap(true){};

	TextboxProperty(
		std::shared_ptr<FormatF> fontAndColor,
		std::shared_ptr<SolidLine> line,
		std::shared_ptr<SolidLine> editLine,
		std::shared_ptr<SolidLine> focusedLine,
		std::shared_ptr<SolidFill> normalFill,
		std::shared_ptr<SolidFill> focusedFill,
		std::shared_ptr<SolidFill> selectedFill,
		std::shared_ptr<SolidFill> unfocusSelectedFill,
		std::shared_ptr<SolidFill> hotFill,
		std::shared_ptr<CRectF> padding)
		:Format(fontAndColor),
		Line(line),
		EditLine(editLine),
		FocusedLine(focusedLine),
		NormalFill(normalFill),
		FocusedFill(focusedFill),
		SelectedFill(selectedFill),
		UnfocusSelectedFill(unfocusSelectedFill),
		HotFill(hotFill),
		Padding(padding),
		BlankLine(std::make_shared<SolidLine>(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.6f, 1.0f)),
		VScrollPropPtr(std::make_shared<ScrollProperty>()),
		HScrollPropPtr(std::make_shared<ScrollProperty>()),
		IsWrap(true){};

	virtual ~TextboxProperty() = default;

	template <class Archive>
    void save(Archive& ar)
    {
		ar("Format",Format);
		ar("Line",Line);
		ar("EditLine", EditLine);
		ar("FocusedLine",FocusedLine);
		ar("BlankLine", BlankLine);
		ar("NormalFill",NormalFill);
		ar("FocusedFill",FocusedFill);
		ar("SelectedFill",SelectedFill);
		ar("UnfocusSelectedFill", UnfocusSelectedFill);
		ar("HotFill", HotFill);
		ar("Padding",Padding);
		ar("VScrollProperty", VScrollPropPtr);
		ar("HScrollProperty", HScrollPropPtr);
		ar("IsWrap", IsWrap);
	}

	template <class Archive>
    void load(Archive& ar)
    {
		ar("Format",Format);
		ar("Line",Line);
		ar("EditLine", EditLine);
		ar("FocusedLine",FocusedLine);
		ar("BlankLine", BlankLine);
		ar("NormalFill",NormalFill);
		ar("FocusedFill",FocusedFill);
		ar("SelectedFill",SelectedFill);
		ar("UnfocusSelectedFill", UnfocusSelectedFill);
		ar("HotFill", HotFill);
		ar("Padding",Padding);
		ar("VScrollProperty", VScrollPropPtr);
		ar("HScrollProperty", HScrollPropPtr);
		ar("IsWrap", IsWrap);
	}

	friend void to_json(json& j, const TextboxProperty& o);
    friend void from_json(const json& j, TextboxProperty& o);
};

void to_json(json& j, const TextboxProperty& o)
{
	j = json{
		{"Format",o.Format},
		{"Line",o.Line},
		{"EditLine", o.EditLine},
		{"FocusedLine",o.FocusedLine},
		{"BlankLine", o.BlankLine},
		{"NormalFill",o.NormalFill},
		{"FocusedFill",o.FocusedFill},
		{"SelectedFill",o.SelectedFill},
		{"UnfocusSelectedFill", o.UnfocusSelectedFill},
		{"HotFill", o.HotFill},
		{"Padding",o.Padding},
		{"VScrollProperty", o.VScrollPropPtr},
		{"HScrollProperty", o.HScrollPropPtr},
		{"IsWrap", o.IsWrap }
	};
}

void from_json(const json& j, TextboxProperty& o)
{
	j.at("Format").get_to(o.Format);
	j.at("Line").get_to(o.Line);
	j.at("EditLine").get_to(o.EditLine);
	j.at("FocusedLine").get_to(o.FocusedLine);
	j.at("BlankLine").get_to(o.BlankLine);
	j.at("NormalFill").get_to(o.NormalFill);
	j.at("FocusedFill").get_to(o.FocusedFill);
	j.at("SelectedFill").get_to(o.SelectedFill);
	j.at("UnfocusSelectedFill").get_to(o.UnfocusSelectedFill);
	j.at("HotFill").get_to(o.HotFill);
	j.at("Padding").get_to(o.Padding);
	j.at("VScrollProperty").get_to(o.VScrollPropPtr);
	j.at("HScrollProperty").get_to(o.HScrollPropPtr);
	j.at("IsWrap").get_to(o.IsWrap);
}



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

	friend void to_json(json& j, const SyntaxAppearance& o);
    friend void from_json(const json& j, SyntaxAppearance& o);
};
void to_json(json& j, const SyntaxAppearance& o)
{
	j = json{
		{"Regex", o.Regex},
		{"SyntaxFormat", o.SyntaxFormat}
	};
}
void from_json(const json& j, SyntaxAppearance& o)
{
	j.at("Regex").get_to(o.Regex);
	j.at("SyntaxFormat").get_to(o.SyntaxFormat);
}


struct TextEditorProperty :public TextboxProperty
{
	ReactiveVectorProperty<std::tuple<SyntaxAppearance>> SyntaxAppearances;
	TextEditorProperty():TextboxProperty(){}

	template <class Archive>
	void save(Archive& ar)
	{
		TextboxProperty::save(ar);
		ar("SyntaxAppearances", SyntaxAppearances);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		TextboxProperty::load(ar);
		ar("SyntaxAppearances", SyntaxAppearances);

		if (SyntaxAppearances.empty()) {
			SyntaxAppearances.push_back(
				std::make_tuple(
				SyntaxAppearance(L"/\\*.*?\\*/",
				SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false))));
			SyntaxAppearances.push_back(
				std::make_tuple(
				SyntaxAppearance(L"//.*?\n",
				SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false))));

		}
	}

	friend void to_json(json& j, const TextEditorProperty& o);
    friend void from_json(const json& j, TextEditorProperty& o);
};
void to_json(json& j, const TextEditorProperty& o)
{
	to_json(j, static_cast<const TextboxProperty&>(o));
	j["SyntaxAppearances"] = o.SyntaxAppearances;
}

void from_json(const json& j, TextEditorProperty& o)
{
	from_json(j, static_cast<TextboxProperty&>(o));
	j.at("SyntaxAppearances").get_to(o.SyntaxAppearances);

	if (o.SyntaxAppearances.empty()) {
		o.SyntaxAppearances.push_back(
			std::make_tuple(
			SyntaxAppearance(L"/\\*.*?\\*/",
			SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false))));
		o.SyntaxAppearances.push_back(
			std::make_tuple(
			SyntaxAppearance(L"//.*?\n",
			SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false))));
	}
}

struct CellProperty :public TextboxProperty
{
public:
	using TextboxProperty::TextboxProperty;
};


struct HeaderProperty:public CellProperty
{
public:
	std::shared_ptr<SolidLine> DragLine;
public:
	HeaderProperty()
		:CellProperty(
		std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f),
		std::make_shared<SolidLine>(211.f / 255.f, 206.f / 255.f, 188.f / 255.f, 1.0f, 1.0f),
		std::make_shared<SolidLine>(0.f / 255.f, 0.f / 255.f, 255.f / 255.f, 1.0f, 1.0f),
		std::make_shared<SolidLine>(22.f / 255.f, 160.f / 255, 133.f / 255.f, 1.0f, 1.0f),
		std::make_shared<SolidFill>(239.f / 255.f, 239.f / 255, 232.f / 255.f, 1.0f),
		std::make_shared<SolidFill>(195.f / 255.f, 224.f / 255, 226.f / 255.f, 1.0f),
		std::make_shared<SolidFill>(195.f / 255.f, 224.f / 255, 226.f / 255.f, 0.3f),
		std::make_shared<SolidFill>(224.f / 255.f, 224.f / 255, 224.f / 255.f, 0.3f),
		std::make_shared<SolidFill>(1.0f, 1.0f, 1.0f, 0.3f),
		std::make_shared<CRectF>(2.0f, 2.0f, 2.0f, 2.0f)),
		DragLine(std::make_shared<SolidLine>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f)){};
	virtual ~HeaderProperty(){}

    template <class Archive>
    void save(Archive& ar)
    {
		CellProperty::save(ar);
    }
    template <class Archive>
    void load(Archive& ar)
    {
		CellProperty::load(ar);
    }
};
