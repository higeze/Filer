#include "Row.h"
#include "GridView.h"
#include "CellProperty.h"
#include "D2DWWindow.h"

FLOAT CRow::GetStart() { return m_start/* + m_pGrid->GetRectInWnd().top*/; }

FLOAT CRow::GetEnd() { return m_start + m_length /* + +m_pGrid->GetRectInWnd().top*/; }

CRectF CRow::GetRectInWnd()
{
	auto visCols = m_pGrid->GetContainer<ColTag, VisTag>();
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
			m_pGrid->GetWndPtr()->GetDirectPtr()->GetVirtualHeight(*(m_pGrid->GetCellProperty()->Format)) +
			m_pGrid->GetCellProperty()->Line->Width * 0.5f * 2.f +
			m_pGrid->GetCellProperty()->Padding->top +
			m_pGrid->GetCellProperty()->Padding->bottom;
		m_isVirtualMeasureValid = true;
	}
	return m_length;
}

FLOAT CRow::GetFitLength()
{
	if (!m_isFitMeasureValid) {
		m_fitLength = m_pGrid->GetRowHeight(this);
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
	m_pGrid->OnRowPropertyChanged(this, name);
}

void CRow::RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc)
{
	pDirect->FillSolidRectangle(*(m_spCellProperty->NormalFill), rc);
}

void CRow::RenderHighlight(CDirect2DWrite* pDirect, const CRectF& rc)
{
	if (GetIsSelected() && m_pGrid->GetIsFocused()) {
		pDirect->FillSolidRectangle(*(m_spCellProperty->SelectedFill), rc);
	} else if (GetIsSelected()) {
		pDirect->FillSolidRectangle(*(m_spCellProperty->UnfocusSelectedFill), rc);
	}
}

//void CRow::OnPaint(const PaintEvent& e)
//{
//	CRectF rcPaint(GetRectInWnd());
//	e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);
//
//	if (GetIsSelected() && m_pGrid->GetIsFocused()  /*::GetFocus() == m_pGrid->m_hWnd*/) {
//		e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_spCellProperty->SelectedFill), rcPaint);
//	} else if (GetIsSelected()) {
//		e.WndPtr->GetDirectPtr()->FillSolidRectangle(*(m_spCellProperty->UnfocusSelectedFill), rcPaint);
//	}
//}
