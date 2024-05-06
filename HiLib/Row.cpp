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
		std::shared_ptr<CCell> cell = m_pGrid->Cell<VisTag>(GetIndex<VisTag>() + m_pGrid->GetFrozenCount<RowTag>(), m_pGrid->GetFrozenCount<ColTag>());
		m_length =
			m_pGrid->GetWndPtr()->GetDirectPtr()->GetVirtualHeight(cell->GetFormat()) +
			cell->GetNormalBorder().Width * 0.5f * 2.f +
			cell->GetPadding().top +
			cell->GetPadding().bottom;
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

//void CRow::RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc)
//{
//	auto cell = m_pGrid->Cell<VisTag>(GetIndex<VisTag>() + m_pGrid->GetFrozenCount<RowTag>(), m_pGrid->GetFrozenCount<ColTag>());
//	pDirect->FillSolidRectangle(cell->GetNormalBackground(), rc);
//}

//void CRow::RenderHighlight(CDirect2DWrite* pDirect, const CRectF& rc)
//{
//	auto cell = m_pGrid->Cell<VisTag>(GetIndex<VisTag>() + m_pGrid->GetFrozenCount<RowTag>(), m_pGrid->GetFrozenCount<ColTag>());
//	if (GetIsSelected() && m_pGrid->GetIsFocused()) {
//		pDirect->FillSolidRectangle(cell->GetSelectedOverlay(), rc);
//	} else if (GetIsSelected()) {
//		pDirect->FillSolidRectangle(cell->GetUnfocusSelectedOverlay(), rc);
//	}
//}

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
