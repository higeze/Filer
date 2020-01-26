#include "FilterCell.h"
#include "Sheet.h"
#include "CellProperty.h"
#include "Column.h"
#include "GridView.h"
#include "Textbox.h"

CFilterCell::CFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu)
	:CEditableCell(pSheet, pRow, pColumn, spProperty,pMenu){ }

CFilterCell::~CFilterCell(){}

std::wstring CFilterCell::GetString()
{
	return m_pColumn->GetFilter();
}

void CFilterCell::SetString(const std::wstring& str)
{
	//Filter cell undo redo is set when Post WM_FILTER
	if(GetString()!=str){
		CCell::SetStringNotify(str);
		m_deadlinetimer.run([hWnd = m_pSheet->GetGridPtr()->m_hWnd, newString = str] {
			::PostMessage(hWnd,WM_FILTER,NULL,NULL);
		}, std::chrono::milliseconds(200));
	}
}

void CFilterCell::SetStringCore(const std::wstring& str)
{
	m_pColumn->SetFilter(str);
}

void CFilterCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{

	std::wstring str = GetString();
	if (!str.empty()) {
		pDirect->DrawTextLayout(*(m_spProperty->Format), str, rcPaint);
	}
	else {
		str = L"Filter items...";
		d2dw::FormatF filterFnC(
			m_spProperty->Format->Font.FamilyName, m_spProperty->Format->Font.Size,
			210.0f / 255, 210.0f / 255, 210.0f / 255, 1.0f);
		pDirect->DrawTextLayout(filterFnC, str, rcPaint);
	}
}
