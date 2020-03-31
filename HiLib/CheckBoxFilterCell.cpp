#include "CheckBoxFilterCell.h"
#include "MyRect.h"
#include "MyDC.h"
#include "Sheet.h"
#include "GridView.h"
#include "Column.h"
//

CCheckBoxFilterCell::CCheckBoxFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CCheckBoxCell(pSheet, pRow, pColumn, spProperty)
{
	SetCheckBoxType(CheckBoxType::ThreeState);
}

CheckBoxState CCheckBoxFilterCell::GetCheckBoxState() const
{ 
	return Str2State(m_pColumn->GetFilter());
}

void CCheckBoxFilterCell::SetCheckBoxState(const CheckBoxState& state)
{ 
	return m_pColumn->SetFilter(State2Str(state));
}

std::wstring CCheckBoxFilterCell::GetString()
{
	return m_pColumn->GetFilter();
}

void CCheckBoxFilterCell::SetStringCore(const std::wstring& str)
{
	m_pColumn->SetFilter(str);
}

void CCheckBoxFilterCell::SetString(const std::wstring& str)
{
	//Filter cell undo redo is set when Post WM_FILTER
	if (GetString() != str) {
		std::wstring newString = str;
		CCell* pCell = this;
		HWND hWnd = m_pSheet->GetGridPtr()->m_hWnd;
		m_deadlinetimer.run([hWnd, pCell, newString] {
			pCell->CCell::SetString(newString);
			::PostMessage(hWnd, WM_FILTER, NULL, NULL);
			}, std::chrono::milliseconds(200));
	}
}

void CCheckBoxFilterCell::OnPropertyChanged(const wchar_t* name)
{
	if (!_tcsicmp(L"value", name)) {
		//Update valid flag
		m_isFitMeasureValid = false;
		m_bActMeasureValid = false;
	} else if (!_tcsicmp(L"size", name)) {
		m_bActMeasureValid = false;
	}
	//Notify to Row, Column and Sheet
	m_pRow->OnCellPropertyChanged(this, name);
	m_pColumn->OnCellPropertyChanged(this, name);
}
