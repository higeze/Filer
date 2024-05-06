#include "EditorTextBox.h"
#include "EditorScroll.h"
#include "Editor.h"
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
	m_pVScroll->ScrollChanged.connect([this]() { ClearHighliteRects(); });
}

void CEditorTextBox::PaintHighlite(const PaintEvent& e)
{
	//Draw Highlite
	for (auto rc : GetHighliteRects()) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetFindHighliteOverlay(), rc);
	}
}

void CEditorTextBox::LoadHighliteRects()
{
	//Find Highlight
	m_optHighliteRects = std::vector<CRectF>();
	const auto pEditor = static_cast<CEditor*>(GetParentControlPtr());
	const auto spFilter = pEditor->GetFilterBoxPtr();
	const auto& find = *spFilter->Text;

	auto subStrSize = find.size();
	auto pos = *Text | find_ignorecase(find);
	const auto& actualCharRects = GetActualCharRects();
 
	while (subStrSize != 0 && pos != std::wstring::npos) {
		m_optHighliteRects->emplace_back(
			actualCharRects[pos].left,
			actualCharRects[pos].top,
			actualCharRects[pos + subStrSize - 1].right,
			actualCharRects[pos].bottom
		);
		pos = *Text | find_ignorecase(find, pos + subStrSize);
	}
}

void CEditorTextBox::ClearHighliteRects()
{
	m_optHighliteRects = std::nullopt;
	static_cast<CEditorVScroll*>(m_pVScroll.get())->ClearHighliteRects();
}

void CEditorTextBox::UpdateAll()
{
	CTextBox::UpdateAll();
	ClearHighliteRects();
}