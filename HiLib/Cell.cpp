#include "Cell.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"
#include "Row.h"
#include "Column.h"
#include "RowColumn.h"
#include "MyMenu.h"

bool CCell::operator<(CCell& rhs)
{
	return _tcsicmp(this->GetSortString().c_str(), rhs.GetSortString().c_str())>0;
}
bool CCell::operator>(CCell& rhs)
{
	return _tcsicmp(this->GetSortString().c_str(), rhs.GetSortString().c_str())<0;
}

d2dw::CSizeF CCell::GetInitSize(d2dw::CDirect2DWrite* pDirect)
{
	return GetFitSize(pDirect);
}

d2dw::CSizeF CCell::GetFitSize(d2dw::CDirect2DWrite* pDirect)
{
	if(!m_isFitMeasureValid){
		m_fitSize = MeasureSize(pDirect);
		m_isFitMeasureValid = true;
	}
	return m_fitSize;
}

d2dw::CSizeF CCell::GetActSize(d2dw::CDirect2DWrite* pDirect)
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

d2dw::CSizeF CCell::MeasureContentSize(d2dw::CDirect2DWrite* pDirect)
{
	return d2dw::CSizeF();
}

d2dw::CSizeF CCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	return d2dw::CSizeF();
}

d2dw::CSizeF CCell::MeasureSize(d2dw::CDirect2DWrite* pDirect)
{
	d2dw::CRectF rcContent(MeasureContentSize(pDirect));

	//Calc CenterBorder Rect
	d2dw::CRectF rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
	m_isFitMeasureValid = true;
	return rcCenter.Size();	
}

d2dw::CSizeF CCell::MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	d2dw::CRectF rcContent(MeasureContentSizeWithFixedWidth(pDirect));

	//Calc CenterBorder Rect
	d2dw::CRectF rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
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

d2dw::CRectF CCell::GetRectInWnd()const
{
	auto rc =  d2dw::CRectF(
		m_pColumn->GetLeft(),
		m_pRow->GetTop(),
		m_pColumn->GetRight(),
		m_pRow->GetBottom());

	return rc;
}

d2dw::CRectF CCell::CenterBorder2InnerBorder(d2dw::CRectF rcCenter)
{
	auto halfLineWidth = m_spCellProperty->Line->Width*0.5f;
	rcCenter-= d2dw::CRectF(halfLineWidth,halfLineWidth, halfLineWidth, halfLineWidth);
	return rcCenter;
}

d2dw::CRectF CCell::InnerBorder2Content(d2dw::CRectF rcInner)
{
	rcInner.DeflateRect(*(m_spCellProperty->Padding));
	return rcInner;
}

d2dw::CRectF CCell::Content2InnerBorder(d2dw::CRectF rcContent)
{
	rcContent.InflateRect(*(m_spCellProperty->Padding));
	return rcContent;
}

d2dw::CRectF CCell::InnerBorder2CenterBorder(d2dw::CRectF rcInner)
{
	//Calc CenterBorder Rect 
	auto halfLineWidth = m_spCellProperty->Line->Width*0.5f;
	rcInner += d2dw::CRectF(halfLineWidth,halfLineWidth, halfLineWidth, halfLineWidth);
	return rcInner;
}

void CCell::PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	//Paint Normal
	pDirect->FillSolidRectangle(*(m_spCellProperty->NormalFill), rcPaint);
	//Selected
	if (GetIsSelected() && m_pSheet->GetIsFocused()  /*::GetFocus() == m_pSheet->GetGridPtr()->m_hWnd*/) {
		pDirect->FillSolidRectangle(*(m_spCellProperty->SelectedFill), rcPaint);
	} else if (GetIsSelected()) {
		pDirect->FillSolidRectangle(*(m_spCellProperty->UnfocusSelectedFill), rcPaint);
	}
	//Hot, Pressed
	if (m_state == UIElementState::Hot || m_state == UIElementState::Pressed) {
		pDirect->FillSolidRectangle(*(m_spCellProperty->HotFill), rcPaint);
	}
}

void CCell::PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	pDirect->DrawSolidRectangle(*(m_spCellProperty->Line), rcPaint);
}

void CCell::PaintFocus(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	if(GetIsFocused()){
		auto halfLineWidth = m_spCellProperty->Line->Width*0.5f;
		rcPaint.DeflateRect(halfLineWidth, halfLineWidth);
		pDirect->DrawSolidRectangle(*(m_spCellProperty->FocusedLine), rcPaint);
	}
}

void CCell::OnPaint(const PaintEvent& e)
{
	d2dw::CRectF rcClient(GetRectInWnd());
	d2dw::CRectF rcInner(CenterBorder2InnerBorder(rcClient));
	d2dw::CRectF rcContent(InnerBorder2Content(rcInner));
	PaintLine(e.WndPtr->GetDirectPtr(),rcClient);
	PaintBackground(e.WndPtr->GetDirectPtr(),rcInner);
	PaintContent(e.WndPtr->GetDirectPtr(),rcContent);
	PaintFocus(e.WndPtr->GetDirectPtr(), rcInner);
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
	return m_pRow->GetVisible() && m_pColumn->GetVisible();
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

CWnd* CCell::GetWndPtr()const { return m_pSheet->GetWndPtr(); }



