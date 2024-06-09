#pragma once
#include "Textbox.h"
#include "getter_macro.h"

struct ExecutableInfo
{
	std::wstring Link = L"";
	UINT32 StartPosition = 0;
	UINT32 Length = 0;
};

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


class CColoredTextBox:public CTextBox
{
public:
	const std::vector<SyntaxAppearance>& GetSyntaxAppearances() const
	{
		static const std::vector<SyntaxAppearance> value{
			SyntaxAppearance(L"/\\*.*?\\*/", SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false, false)),
			SyntaxAppearance(L"//.*?\n", SyntaxFormatF(CColorF(0.0f, 0.5f, 0.0f), false, false))}; return value;
	}
	const std::vector<ExecutableAppearance>& GetExecutableAppearances() const
	{
		static const std::vector<ExecutableAppearance> value{
			ExecutableAppearance{
				LR"((?:[a-zA-Z]:|\\\\[a-zA-Z0-9_.$Åú-]+\\[a-zA-Z0-9_.$Åú-]+)\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]*)",
				SyntaxFormatF(CColorF(0.0f, 0.0f, 1.0f, 1.0f), false, true)
				},
				ExecutableAppearance{
				LR"(https?://[\w!?/+\-_~;.,*&@#$%()'[\]=]+)",
				SyntaxFormatF(CColorF(0.0f, 0.0f, 1.0f, 1.0f), false, true)
				},
				ExecutableAppearance{
				LR"(Notes://[\w!?/+\-_~;.,*&@#$%()'[\]=]+)",
				SyntaxFormatF(CColorF(0.0f, 0.0f, 1.0f, 1.0f), false, true)
				}		
		}; return value;
	}

protected:
	std::vector<ExecutableInfo> m_executableInfos;

public:
	CColoredTextBox()
		: CTextBox(){}

	CColoredTextBox(
		CD2DWControl* pParentControl,
		const std::wstring& text):
		CTextBox(pParentControl, text){}
	CColoredTextBox(
		CD2DWControl* pParentControl,
		std::unique_ptr<CVScroll>&& pVScroll,
		std::unique_ptr<CHScroll>&& pHScroll,
		const std::wstring& text)
		:CTextBox(m_pParentControl, 
		std::forward<std::unique_ptr<CVScroll>>(pVScroll),
		std::forward<std::unique_ptr<CHScroll>>(pHScroll), text){}
	virtual ~CColoredTextBox() = default;

	void LoadTextLayoutPtr() override;
	void Normal_LButtonDown(const LButtonDownEvent& e) override;
	void Normal_SetCursor(const SetCursorEvent& e) override;

    template<class Archive>
    void save(Archive & archive) const
    {
		archive(cereal::base_class<CD2DWControl>(this));
    }
    template<class Archive>
    void load(Archive & archive)
    {
		archive(cereal::base_class<CD2DWControl>(this));
    }

	friend void to_json(json& j, const CColoredTextBox& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));
	}

	friend void from_json(const json& j, CColoredTextBox& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));
	}
};
