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
	using CTextBox::CTextBox;
	virtual ~CColoredTextBox() = default;

	void LoadTextLayoutPtr() override;
	void Normal_LButtonDown(const LButtonDownEvent& e) override;
	void Normal_SetCursor(const SetCursorEvent& e) override;
};
