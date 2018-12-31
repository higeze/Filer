#include "Row.h"
#include "GridView.h"
#include "SheetCell.h"
#include "CellProperty.h"
#include "SheetEventArgs.h"

CBand::coordinates_type CRow::GetHeight()
{
	if(!m_bMeasureValid){
		m_height = m_pSheet->GetRowHeight(this);
		m_bMeasureValid=true;
	}
	return m_height;
}

void CRow::SetHeight(const coordinates_type& height)
{
	if(m_height!=height){
		m_height=height;
		OnPropertyChanged(L"height");
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
	if (!_tcsicmp(L"value", name)) {
		if (pCell->GetLineType() == LineType::MultiLine) {
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


//CRow::size_type CRow::GetIndex<VisTag>()const
//{
//	return m_pSheet->Pointer2Index<RowTag, VisTag>(this);
//}
//CRow::size_type CRow::GetIndex<AllTag>()const
//{
//	return m_pSheet->Pointer2Index<RowTag, AllTag>(this);
//}

CParentRow::CParentRow(CGridView* pGrid):CRow(pGrid){} 

CParentRow::coordinates_type CParentRow::Offset()const
{
	coordinates_type offset(static_cast<CGridView*>(m_pSheet)->GetPaintRect().top);
	//if(m_pSheet->Visible() && GetVisible() && GetIndex<AllTag>()>=0){
	if(m_pSheet->Visible() && GetVisible() && GetIndex<VisTag>()>=0){
		offset-=static_cast<CGridView*>(m_pSheet)->GetVerticalScrollPos();
	}
	return offset;
}

CChildRow::CChildRow(CSheetCell* pSheetCell):CRow(pSheetCell){} 

CChildRow::coordinates_type CChildRow::Offset()const
{
	return static_cast<CSheetCell*>(m_pSheet)->GetTop()
			+ (coordinates_type)floor(static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->GetPenPtr()->GetWidth()*0.5)
			+ (coordinates_type)floor(static_cast<CSheetCell*>(m_pSheet)->GetHeaderPropertyPtr()->GetPenPtr()->GetWidth()*0.5)
			+ static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->GetPadding().top;
}