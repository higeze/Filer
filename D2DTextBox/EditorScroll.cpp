#include "EditorScroll.h"
#include "EditorTextBox.h"

/******************/
/* CEditorVScroll */
/******************/
void CEditorVScroll::PaintForeground(const PaintEvent& e)
{
	for (const auto& rc : GetHighliteRects()) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetFindHighlite(), rc);
	}
}

CRectF CEditorVScroll::GetHighliteRangeRect()const
{
	auto highliteRangeRect = GetThumbRangeRect();
	highliteRangeRect.left += kHighliteOffset;
	highliteRangeRect.right -= kHighliteOffset;
	return highliteRangeRect;
}

void CEditorVScroll::LoadHighliteRects()
{
	//Find Highlight
	m_optHighliteRects = std::vector<CRectF>();

	const auto pEditTextBox = static_cast<CEditorTextBox*>(m_pParentControl);
	const auto& highliteRangeRect = GetHighliteRangeRect();
	const auto& textboxRect = pEditTextBox->GetActualContentRect();
	const auto& textHighliteRects = pEditTextBox->GetHighliteRects();

	for (auto iter = textHighliteRects.cbegin(); iter != textHighliteRects.cend(); ++iter) {
		if (iter ==  textHighliteRects.cbegin() || std::prev(iter)->top != iter->top) {
			m_optHighliteRects->emplace_back(
				highliteRangeRect.left,
				highliteRangeRect.top + highliteRangeRect.Height() * (iter->top - textboxRect.top) / textboxRect.Height(),
				highliteRangeRect.right,
				highliteRangeRect.top + highliteRangeRect.Height() * (iter->bottom - textboxRect.top) / textboxRect.Height()
			);
		}
	}
}
