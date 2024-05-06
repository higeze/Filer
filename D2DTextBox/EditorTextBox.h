#pragma once
#include "ColoredTextbox.h"
#include "getter_macro.h"

class CEditorTextBox:public CColoredTextBox
{
	LAZY_GETTER_NO_CLEAR_IMPL(std::vector<CRectF>, HighliteRects)

public:

	const SolidFill& GetFindHighliteOverlay()const
	{
		static const SolidFill value(244.f / 255, 167.f / 255, 33.f / 255, 100.f / 255); return value;
	}

public:
	CEditorTextBox(
		CD2DWControl* pParentControl,
		const std::wstring& text);
	virtual ~CEditorTextBox() = default;
	void PaintHighlite(const PaintEvent& e) override;
	void UpdateAll() override;


};
