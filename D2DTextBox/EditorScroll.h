#pragma once
#include "Scroll.h"
#include "getter_macro.h"

struct EditorScrollProperty;

class CEditorVScroll :public CVScroll
{
	LAZY_GETTER(std::vector<CRectF>, HighliteRects)
	const FLOAT kHighliteOffset = 2.f;
public:
	CEditorVScroll(CD2DWControl* pParentControl, const std::shared_ptr<EditorScrollProperty>& spScrollProp)
		:CVScroll(pParentControl, std::static_pointer_cast<ScrollProperty>(spScrollProp)){}
	virtual ~CEditorVScroll() = default;
	virtual CRectF GetHighliteRangeRect()const;
	virtual void PaintForeground(const PaintEvent& e) override;
};
