#include "Row.h"
#include "GridView.h"
#include "CellProperty.h"
#include "D2DWWindow.h"

FLOAT CRow::GetStart() { return m_start/* + m_pSheet->GetRectInWnd().top*/; }

FLOAT CRow::GetEnd() { return m_start + m_length /* + +m_pSheet->GetRectInWnd().top*/; }

CRectF CRow::GetRectInWnd()
{
	auto visCols = m_pSheet->GetContainer<ColTag, VisTag>();
	return CRectF(visCols.front()->GetLeft(),
		GetTop(),
		visCols.back()->GetRight(),
		GetBottom());
}

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
			m_pSheet->GetWndPtr()->GetDirectPtr()->GetVirtualHeight(*(m_pSheet->GetCellProperty()->Format)) +
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

void CRow::SetIsVisible(const bool& bVisible, bool notify)
{
	if(GetIsVisible()!=bVisible){
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

void CRow::OnPaint(const PaintEvent& e)
{
	CRectF rcPaint(GetRectInWnd());
	e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);

	if (GetIsSelected() && m_pSheet->GetIsFocused()  /*::GetFocus() == m_pSheet->GetGridPtr()->m_hWnd*/) {
		e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_spCellProperty->SelectedFill), rcPaint);
	} else if (GetIsSelected()) {
		e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_spCellProperty->UnfocusSelectedFill), rcPaint);
	}
}
