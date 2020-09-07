#include "FilterCell.h"
#include "Sheet.h"
#include "CellProperty.h"
#include "Column.h"
#include "GridView.h"
#include "Textbox.h"

std::wstring CFilterCell::GetString()
{
	return m_pColumn->GetFilter();
}

void CFilterCell::SetStringCore(const std::wstring& str)
{
	//Filter cell undo redo is set when Post WM_FILTER
	m_deadlinetimer.run([hWnd = m_pSheet->GetWndPtr()->m_hWnd, pColumn = m_pColumn, newString = str]{
		pColumn->SetFilter(newString);
		::PostMessage(hWnd,WM_FILTER,NULL,NULL);
	}, std::chrono::milliseconds(200));
}

void CFilterCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{

	std::wstring str = GetString();
	if (!str.empty()) {
		pDirect->DrawTextLayout(*(m_spCellProperty->Format), str, rcPaint);
	}
	else {
		str = L"Filter items...";
		d2dw::FormatF filterFnC(
			m_spCellProperty->Format->Font.FamilyName, m_spCellProperty->Format->Font.Size,
			210.0f / 255, 210.0f / 255, 210.0f / 255, 1.0f);
		pDirect->DrawTextLayout(filterFnC, str, rcPaint);
	}
}
