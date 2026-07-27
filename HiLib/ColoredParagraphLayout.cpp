#include "ColoredParagraphLayout.h"
#include "ColoredTextLayout.h"
#include "ColoredTextBox.h"


const CComPtr<IDWriteTextLayout1>& CColoredParagraphLayout::GetTextLayoutPtr() const
{
	if (!Text->empty() && !m_pTextLayout) {
		m_pTextLayout = CParagraphLayout::GetTextLayoutPtr();
		if (auto pColoredTextBox = dynamic_cast<CColoredTextBox*>(m_pText->GetTextBoxPtr())) {
			auto pDirect = m_pText->GetTextBoxPtr()->GetWndPtr()->GetDirectPtr();

			//Syntax
			for (const auto& appearance : pColoredTextBox->GetSyntaxAppearances()) {
				if (!appearance.Regex.empty()) {
					auto brush = pDirect->GetColorBrush(appearance.SyntaxFormat.Color);

					std::wsmatch match;
					auto begin = Text->cbegin();
					auto re = std::wregex(appearance.Regex);//L"/\\*.*?\\*/"
					UINT32 beginPos = 0;
					while (std::regex_search(begin, Text->cend(), match, re)) {
						DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
						m_pTextLayout->SetDrawingEffect(brush, range);
						if (appearance.SyntaxFormat.IsBold) {
							m_pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
						}

						begin = match[0].second;
						beginPos = std::distance(Text->cbegin(), begin);
					}
				}
			}

			//Executable
			m_executableInfos.clear();
			for (const auto& apr : pColoredTextBox->GetExecutableAppearances()) {
				auto brush = pDirect->GetColorBrush(apr.SyntaxFormat.Color);
				std::wsmatch match;
				auto begin = Text->cbegin();
				auto re = std::wregex(apr.Regex);
				UINT32 beginPos = 0;
				while (std::regex_search(begin, Text->cend(), match, re)) {
					DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
					m_pTextLayout->SetDrawingEffect(brush, range);
					if (apr.SyntaxFormat.IsBold) {
						m_pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
					}
					m_pTextLayout->SetUnderline(apr.SyntaxFormat.IsUnderline, range);


					m_executableInfos.push_back(ExecutableInfo{ match.str(), range.startPosition, range.length });
					begin = match[0].second;
					beginPos = std::distance(Text->cbegin(), begin);
				}
			}
		}
	}
	return m_pTextLayout;
}

std::optional<ExecutableInfo> CColoredParagraphLayout::HitTestExecutableInfo(UINT32 position) const
{
	auto iter = std::find_if(m_executableInfos.begin(), m_executableInfos.end(), [i = static_cast<UINT32>(position)](const auto& info)->bool
		{
			return i >= info.StartPosition && i < info.StartPosition + info.Length;
		});
	if (iter != m_executableInfos.cend()) {
		return *iter;
	} else {
		return std::nullopt;
	}
}

