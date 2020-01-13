#include "Row.h"
#include "GridView.h"
#include "SheetCell.h"
#include "CellProperty.h"
#include "SheetEventArgs.h"



FLOAT CRow::GetDefaultHeight()
{
	if (!m_bMeasureValid) {
		m_height = 
			m_pSheet->GetGridPtr()->GetDirectPtr()->GetDefaultHeight(*(m_pSheet->GetCellProperty()->Format)) +
			m_pSheet->GetCellProperty()->Line->Width*0.5f + 
			m_pSheet->GetCellProperty()->Padding->top + 
			m_pSheet->GetCellProperty()->Padding->bottom;
	}
	return m_height;
}

FLOAT CRow::GetHeight()
{
	if(!m_bMeasureValid){
		m_height = m_pSheet->GetRowHeight(this);
		m_bMeasureValid=true;
	}
	return m_height;
}

void CRow::SetHeight(const FLOAT height, bool notify)
{
	if(m_height!=height){
		m_height=height;
		if (notify) {
			OnPropertyChanged(L"height");
		}
	}
}
void CRow::SetVisible(const bool& bVisible, bool notify)
{
	if(m_bVisible!=bVisible){
		m_bVisible=bVisible;
		if(notify){
			OnPropertyChanged(L"visible");
		}
	}
}
void CRow::SetSelected(const bool& bSelected)
{
	if(m_bSelected!=bSelected){
		m_bSelected=bSelected;
		OnPropertyChanged(L"selected");
	}
}

void CRow::OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
{
	if (!_tcsicmp(L"value", name) /*|| !_tcsicmp(L"size", name)*/) {
		if (GetSizingType() == SizingType::Depend || GetSizingType() == SizingType::Fit ||
			(GetSizingType() == SizingType::None && (pCell->GetRowSizingType() == SizingType::Depend || pCell->GetRowSizingType() == SizingType::Fit))) {
			m_bMeasureValid = false;
		} else {
			//Do nothing, Cell value change 
		}
	}
}

void CRow::OnPropertyChanged(const wchar_t* name)
{
	m_pSheet->OnRowPropertyChanged(this, name);
}


CParentRow::CParentRow(CGridView* pGrid):CRow(pGrid){} 

CChildRow::CChildRow(CSheetCell* pSheetCell):CRow(pSheetCell){} 
