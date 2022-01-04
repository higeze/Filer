#include "EditorScroll.h"
#include "EditorTextBox.h"
#include "EditorProperty.h"

/******************/
/* CEditorVScroll */
/******************/
void CEditorVScroll::PaintForeground(const PaintEvent& e)
{
	SolidFill FindHighliteFill = SolidFill(244.f / 255, 167.f / 255, 33.f / 255, 1.f); // TODO HIGH
	for (const auto& rc : GetHighliteRects()) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(
			/*std::static_pointer_cast<EditorScrollProperty>(m_spScrollProp)->FindHighliteFill*/FindHighliteFill, rc);
	}
}

CRectF CEditorVScroll::GetHighliteRangeRect()const
{
	auto highliteRangeRect = GetRectInWnd();
	highliteRangeRect.top += m_spScrollProp->ThumbMargin.top;
	highliteRangeRect.bottom -= m_spScrollProp->ThumbMargin.bottom;

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

	for (const auto rc : textHighliteRects) {
		m_optHighliteRects->emplace_back(
			highliteRangeRect.left + highliteRangeRect.Width() * (rc.left - textboxRect.left) / textboxRect.Width(),
			highliteRangeRect.top + highliteRangeRect.Height() * (rc.top - textboxRect.top) / textboxRect.Height(),
			highliteRangeRect.left + highliteRangeRect.Width() * (rc.right - textboxRect.left) / textboxRect.Width(),
			highliteRangeRect.top + highliteRangeRect.Height() * (rc.bottom - textboxRect.top) / textboxRect.Height()
		);
	}
}
