#include "EditorTextBox.h"
#include "EditorScroll.h"
#include "Editor.h"
#include "TextLayout.h"
#include "string_extension.h"

/******************/
/* CEditorTextBox */
/******************/

CEditorTextBox::CEditorTextBox(
	CD2DWControl* pParentControl,
	const std::wstring& text)
	:CColoredTextBox(pParentControl, 
		std::make_unique<CEditorVScroll>(this), 
		std::make_unique<CHScroll>(this),
		text)
{
	//m_pVScroll->ScrollChanged.connect([this]() { ClearHighliteRects(); });
}

void CEditorTextBox::PaintHighlite(const PaintEvent& e)
{
	//Draw Highlite
	const auto pEditor = static_cast<CEditor*>(GetParentControlPtr());
	const auto spFilter = pEditor->GetFilterBoxPtr();
	const auto& find = *spFilter->Text;

	auto fill = GetFindHighliteOverlay();

	auto position = *Text | find_ignorecase(find);
	auto length = find.size();
	while (length != 0 && position != std::wstring::npos) {
		auto rects = m_pTextLayout->HitTestTextRange(position, length);
		for (auto rect : rects) {
			GetWndPtr()->GetDirectPtr()->FillSolidRectangle(fill, rect);
		}
		position = *Text | find_ignorecase(find, position + length);
	}
}