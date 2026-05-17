#include "EditorScroll.h"
#include "Editor.h"
#include "EditorTextbox.h"
#include "TextLayout.h"
#include "string_extension.h"


/******************/
/* CEditorVScroll */
/******************/
void CEditorVScroll::PaintForeground(const PaintEvent& e)
{
	if (auto pTextBox = dynamic_cast<CEditorTextBox*>(GetParentControlPtr())) {
		if (auto pEditor = dynamic_cast<CEditor*>(pTextBox->GetParentControlPtr())) {
			const auto spFilter = pEditor->GetFilterBoxPtr();
			const auto textboxRect = CRectF(0.f,0.f,pTextBox->GetTextPtr()->GetWidth(), pTextBox->GetTextPtr()->GetHeight());
			const auto find = *spFilter->Text;
			const auto highliteRangeRect = GetHighliteRangeRect();

			auto position = (*pTextBox->Text) | find_ignorecase(find);
			auto length = find.size();
			while (length != 0 && position != std::wstring::npos) {
				auto rects = pTextBox->GetTextPtr()->HitTestTextRange(position, length);
				for (auto rect : rects) {
					auto rc = CRectF(
						highliteRangeRect.left,
						highliteRangeRect.top + highliteRangeRect.Height() * (rect.top - textboxRect.top) / textboxRect.Height(),
						highliteRangeRect.right,
						highliteRangeRect.top + highliteRangeRect.Height() * (rect.bottom - textboxRect.top) / textboxRect.Height()
					);
					GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetFindHighlite(), rc);
				}
				position = (*pTextBox->Text) | find_ignorecase(find, position + length);
			}
		}

	}
	
}

CRectF CEditorVScroll::GetHighliteRangeRect()const
{
	auto highliteRangeRect = GetThumbRangeRect();
	highliteRangeRect.left += kHighliteOffset;
	highliteRangeRect.right -= kHighliteOffset;
	return highliteRangeRect;
}
