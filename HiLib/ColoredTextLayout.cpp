#include "ColoredTextLayout.h"
#include "ColoredParagraphLayout.h"

std::optional<ExecutableInfo> CColoredTextLayout::HitTestExecutableInfo(UINT32 position) const
{
	auto [para_index, text_index] = FindParagraphTextPosition(position);
	if (auto para = std::dynamic_pointer_cast<CColoredParagraphLayout>(Paragraphs->at(para_index))) {
		return para->HitTestExecutableInfo(text_index);
	} else {
		return std::nullopt;
	}
}
