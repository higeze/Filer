#include "FilterCell.h"
#include "GridView.h"
#include "CellProperty.h"
#include "Column.h"
#include "GridView.h"
#include "Textbox.h"
#include "D2DWWindow.h"
#include "Dispatcher.h"

std::wstring CFilterCell::GetString()
{
	return m_pColumn->GetFilter();
}

void CFilterCell::SetStringCore(const std::wstring& str)
{
	//Filter cell undo redo is set when Post WM_FILTER
	m_deadlinetimer.run([pWnd = m_pGrid->GetWndPtr(), newString = str, pSheet = m_pGrid, pColumn = m_pColumn]()->void
	{
		pColumn->SetFilter(newString);
		if (auto pGrid = dynamic_cast<CGridView*>(pSheet)) {
			pWnd->GetDispatcherPtr()->PostInvoke([pGrid]()->void { pGrid->Filter(); });
		}
	}, std::chrono::milliseconds(200));
}

void CFilterCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{

	std::wstring str = GetString();
	if (!str.empty()) {
		pDirect->DrawTextLayout(*(m_spCellProperty->Format), str, rcPaint);
	}
	else {
		str = L"Filter items...";
		FormatF filterFnC(
			m_spCellProperty->Format->Font.FamilyName, m_spCellProperty->Format->Font.Size,
			210.0f / 255, 210.0f / 255, 210.0f / 255, 1.0f);
		pDirect->DrawTextLayout(filterFnC, str, rcPaint);
	}
}
