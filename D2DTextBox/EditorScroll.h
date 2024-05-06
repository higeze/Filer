#pragma once
#include "Scroll.h"
#include "getter_macro.h"

class CEditorVScroll :public CVScroll
{
	LAZY_GETTER(std::vector<CRectF>, HighliteRects)
	const FLOAT kHighliteOffset = 2.f;
public:
	const SolidFill& GetFindHighlite() const
	{
		static const SolidFill value(244.f / 255, 167.f / 255, 33.f / 255, 1.f); return value;
	}

public:
	CEditorVScroll(CD2DWControl* pParentControl)
		:CVScroll(pParentControl){}
	virtual ~CEditorVScroll() = default;
	virtual CRectF GetHighliteRangeRect()const;
	virtual void PaintForeground(const PaintEvent& e) override;
};
