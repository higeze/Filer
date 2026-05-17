#pragma once
#include "ParagraphLayout.h"
#include "ColoredTextBox.h"

class CColoredParagraphLayout :public CParagraphLayout
{
private:
	mutable std::vector<ExecutableInfo> m_executableInfos;
public:
	using CParagraphLayout::CParagraphLayout;

	virtual const CComPtr<IDWriteTextLayout1>& GetTextLayoutPtr() const;
	std::optional<ExecutableInfo> HitTestExecutableInfo(UINT32 position) const;
};
