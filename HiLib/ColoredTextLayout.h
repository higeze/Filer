#pragma once
#include "TextLayout.h"
#include "ColoredParagraphLayout.h"

class CColoredTextLayout : public CTextLayout
{
public:
	using CTextLayout::CTextLayout;

	virtual void SubscribeTextChange(const reactive_wstring_ptr::notify_type& notify)
	{
		SubscribeTextChangeT<CColoredParagraphLayout>(notify);
	}

	std::optional<ExecutableInfo> HitTestExecutableInfo(UINT32 position) const;

};
