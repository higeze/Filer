#include "Cell.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"
#include "Row.h"
#include "Column.h"
#include "RowColumn.h"
#include "MyMenu.h"
#include "D2DWWindow.h"

bool CCell::operator<(CCell& rhs)
{
	return _tcsicmp(this->GetSortString().c_str(), rhs.GetSortString().c_str())>0;
}
bool CCell::operator>(CCell& rhs)
{
	return _tcsicmp(this->GetSortString().c_str(), rhs.GetSortString().c_str())<0;
}

CSizeF CCell::GetInitSize(CDirect2DWrite* pDirect)
{
	return GetFitSize(pDirect);
}

CSizeF CCell::GetFitSize(CDirect2DWrite* pDirect)
{
	if(!m_isFitMeasureValid){
		m_fitSize = MeasureSize(pDirect);
		m_isFitMeasureValid = true;
	}
	return m_fitSize;
}

CSizeF CCell::GetActSize(CDirect2DWrite* pDirect)
{
	if(!m_isActMeasureValid){
		auto width = m_pColumn->GetWidth();
		auto fitSize = GetFitSize(pDirect);
		if(fitSize.width <= width){
			m_isActMeasureValid = true;
			m_actSize.width = width;
			m_actSize.height = m_fitSize.height;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(pDirect);
		}
	}
	return m_actSize;
}

CSizeF CCell::MeasureContentSize(CDirect2DWrite* pDirect)
{
	return CSizeF();
}

CSizeF CCell::MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	return CSizeF();
}

CSizeF CCell::MeasureSize(CDirect2DWrite* pDirect)
{
	CRectF rcContent(MeasureContentSize(pDirect));

	//Calc CenterBorder Rect
	CRectF rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
	m_isFitMeasureValid = true;
	return rcCenter.Size();	
}

CSizeF CCell::MeasureSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	CRectF rcContent(MeasureContentSizeWithFixedWidth(pDirect));

	//Calc CenterBorder Rect
	CRectF rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
	m_isActMeasureValid = true;
	return rcCenter.Size();	
}

void CCell::OnPropertyChanged(const wchar_t* name)
{
	if (!_tcsicmp(L"value", name)) {
		//Update valid flag
		m_isFitMeasureValid = false;
		m_isActMeasureValid = false;
	} else if (!_tcsicmp(L"size", name)) {
		m_isFitMeasureValid = false;
		m_isActMeasureValid = false;
	}
	//Notify to Row, Column and Sheet
	m_pRow->OnCellPropertyChanged(this, name);
	m_pColumn->OnCellPropertyChanged(this, name);
	m_pSheet->OnCellPropertyChanged(this, name);
}

FLOAT CCell::GetLeft()const
{
	return m_pColumn->GetLeft();
}

FLOAT CCell::GetTop()const
{
	return m_pRow->GetTop();
}

CRectF CCell::GetRectInWnd()const
{
	auto rc =  CRectF(
		m_pColumn->GetLeft(),
		m_pRow->GetTop(),
		m_pColumn->GetRight(),
		m_pRow->GetBottom());

	return rc;
}

CRectF CCell::CenterBorder2InnerBorder(CRectF rcCenter)
{
	auto halfLineWidth = m_spCellProperty->Line->Width*0.5f;
	rcCenter-= CRectF(halfLineWidth,halfLineWidth, halfLineWidth, halfLineWidth);
	return rcCenter;
}

CRectF CCell::InnerBorder2Content(CRectF rcInner)
{
	rcInner.DeflateRect(*(m_spCellProperty->Padding));
	return rcInner;
}

CRectF CCell::Content2InnerBorder(CRectF rcContent)
{
	rcContent.InflateRect(*(m_spCellProperty->Padding));
	return rcContent;
}

CRectF CCell::InnerBorder2CenterBorder(CRectF rcInner)
{
	//Calc CenterBorder Rect 
	auto halfLineWidth = m_spCellProperty->Line->Width*0.5f;
	rcInner += CRectF(halfLineWidth,halfLineWidth, halfLineWidth, halfLineWidth);
	return rcInner;
}

void CCell::PaintBackground(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	//PaintNormalBackground(pDirect, rcPaint);
	//PaintSelectedBackground(pDirect, rcPaint);
	PaintHotBackground(pDirect, rcPaint);
}

void CCell::PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	//pDirect->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);
}

void CCell::PaintSelectedBackground(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	//if (GetIsSelected() && m_pSheet->GetIsFocused()  /*::GetFocus() == m_pSheet->GetGridPtr()->m_hWnd*/) {
	//	pDirect->FillSolidRectangle(*(m_spCellProperty->SelectedFill), rcPaint);
	//} else if (GetIsSelected()) {
	//	pDirect->FillSolidRectangle(*(m_spCellProperty->UnfocusSelectedFill), rcPaint);
	//}
}

void CCell::PaintHotBackground(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	if (m_state == UIElementState::Hot || m_state == UIElementState::Pressed) {
		pDirect->FillSolidRectangle(*(m_spCellProperty->HotFill), rcPaint);
	}
}

void CCell::PaintLine(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	pDirect->DrawSolidRectangleByLine(*(m_spCellProperty->Line), rcPaint);
}

void CCell::PaintFocus(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	if(GetIsFocused()){
		auto halfLineWidth = m_spCellProperty->Line->Width*0.5f;
		rcPaint.DeflateRect(halfLineWidth, halfLineWidth);
		pDirect->DrawSolidRectangleByLine(*(m_spCellProperty->FocusedLine), rcPaint);
	}
}

void CCell::OnPaint(const PaintEvent& e)
{
	if (GetRectInWnd().Width() > 0 && GetRectInWnd().Height() > 0) {
		CRectF rcClient(GetRectInWnd());
		CRectF rcInner(CenterBorder2InnerBorder(rcClient));
		CRectF rcContent(InnerBorder2Content(rcInner));
		PaintLine(m_pSheet->GetWndPtr()->GetDirectPtr(), rcClient);
		PaintBackground(m_pSheet->GetWndPtr()->GetDirectPtr(), rcInner);
		PaintContent(m_pSheet->GetWndPtr()->GetDirectPtr(), rcContent);
		PaintFocus(m_pSheet->GetWndPtr()->GetDirectPtr(), rcInner);
	}
}

void CCell::OnLButtonDown(const LButtonDownEvent& e)
{
	CUIElement::OnLButtonDown(e);
	m_pSheet->CellLButtonDown(CellEventArgs(this));
}

void CCell::OnLButtonUp(const LButtonUpEvent& e)
{
	CUIElement::OnLButtonUp(e);
	SetState(UIElementState::Hot);
}

void CCell::OnLButtonDblClk(const LButtonDblClkEvent& e)
{
	m_pSheet->CellLButtonDblClk(CellEventArgs(this));
}

void CCell::OnSetFocus(const SetFocusEvent& e)
{
	OnPropertyChanged(L"focus");
}

void CCell::OnKillFocus(const KillFocusEvent& e)
{
	OnPropertyChanged(L"focus");
}

bool CCell::Filter(const std::wstring& strFilter)
{
	return boost::algorithm::to_lower_copy(GetString()).find(boost::algorithm::to_lower_copy(strFilter))!=std::wstring::npos;
}

bool CCell::GetIsVisible()const
{
	return m_pRow->GetIsVisible() && m_pColumn->GetIsVisible();
}

bool CCell::GetIsSelected()const
{
	return m_bSelected || m_pRow->GetIsSelected() || m_pColumn->GetIsSelected();
}

void CCell::SetIsSelected(const bool& selected)
{
	if(m_bSelected!=selected){
		m_bSelected=selected;
		OnPropertyChanged(L"selected");
	}
}

bool CCell::GetIsFocused()const
{
	return m_pSheet->IsFocusedCell(this) /* || m_pSheet->GetFocused()*/;
}

bool CCell::GetIsDoubleFocused()const
{
	return m_pSheet->IsDoubleFocusedCell(this) /* || m_pSheet->GetFocused()*/;
}


bool CCell::GetIsChecked()const
{
	return m_bChecked;
}

void CCell::SetIsChecked(const bool& bChecked)
{
	if(m_bChecked!=bChecked){
		m_bChecked=bChecked;
		OnPropertyChanged(L"checked");
	}
}

std::wstring CCell::GetString()
{
	return std::wstring();
}

void CCell::SetString(const std::wstring& str, bool notify)
{
	if (GetString() != str) {
		SetStringCore(str);
		if (notify) {
			OnPropertyChanged(L"value");
		}
	}
}



