#include "Row.h"
#include "GridView.h"
#include "CellProperty.h"

FLOAT CRow::GetLength()
{
	if(!m_isMeasureValid){
		FitHeight();
	}

	if (!m_isFitMeasureValid) {
		BoundHeight();
	}
	return m_length;
}

FLOAT CRow::GetVirtualLength()
{
	if (!m_isVirtualMeasureValid) {
		m_length =
			m_pSheet->GetGridPtr()->GetDirectPtr()->GetVirtualHeight(*(m_pSheet->GetCellProperty()->Format)) +
			m_pSheet->GetCellProperty()->Line->Width * 0.5f * 2.f +
			m_pSheet->GetCellProperty()->Padding->top +
			m_pSheet->GetCellProperty()->Padding->bottom;
		m_isVirtualMeasureValid = true;
	}
	return m_length;
}

FLOAT CRow::GetFitLength()
{
	if (!m_isFitMeasureValid) {
		m_fitLength = m_pSheet->GetRowHeight(this);
		m_isFitMeasureValid = true;
	}
	return m_fitLength;
}

void CRow::SetVisible(const bool& bVisible, bool notify)
{
	if(GetVisible()!=bVisible){
		*m_spVisible=bVisible;
		if(notify){
			OnPropertyChanged(L"visible");
		}
	}
}
void CRow::SetIsSelected(const bool& bSelected)
{
	if(m_bSelected!=bSelected){
		m_bSelected=bSelected;
		OnPropertyChanged(L"selected");
	}
}

void CRow::OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
{
	if (!_tcsicmp(L"value", name) || !_tcsicmp(L"size", name)) {
		m_isFitMeasureValid = false;
	}
}

void CRow::OnPropertyChanged(const wchar_t* name)
{
	m_pSheet->OnRowPropertyChanged(this, name);
}