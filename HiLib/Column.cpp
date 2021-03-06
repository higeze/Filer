#include "Column.h"
#include "GridView.h"
#include "SheetCell.h"
#include "CellProperty.h"
#include "SheetEventArgs.h"
#include "SheetEnums.h"

/**
 *  CColumn
 */

FLOAT CColumn::GetWidth()
{
	if (m_isInit) {
		SetWidthWithoutSignal(m_pSheet->GetColumnInitWidth(this));
		m_bMeasureValid = true;
		m_isInit = false;
	} else if (!m_bMeasureValid) {
		switch (GetSizingType()) {
		case SizingType::Fit:
			SetWidthWithoutSignal(m_pSheet->GetColumnFitWidth(this));
			break;
		case SizingType::Depend:
			SetWidthWithoutSignal((std::max)(m_width, m_pSheet->GetColumnFitWidth(this)));
			break;
		case SizingType::Independ:
		default:
			SetWidthWithoutSignal(m_pSheet->GetColumnFitWidth(this));
			break;
		}
		m_bMeasureValid = true;
	}
	return m_width;
}

void CColumn::SetWidth(const FLOAT width)
{
	if(m_width!=width){
		SetWidthWithoutSignal(width);
		//m_pSheet->ColumnWidthChanged(CColumnEventArgs(this));
	}
}

void CColumn::SetWidthWithoutSignal(const FLOAT width) 
{
	m_width = std::clamp(width, m_minWidth, m_maxWidth);
}

void CColumn::SetVisible(const bool& bVisible, bool notify)
{
	if(m_bVisible!=bVisible){
		m_bVisible=bVisible;
		if(notify){
			OnPropertyChanged(L"visible");
		}
	}
}
void CColumn::SetSelected(const bool& bSelected)
{
	if(m_bSelected!=bSelected){
		m_bSelected=bSelected;
		OnPropertyChanged(L"selected");
	}
}
void CColumn::SetSort(const Sorts& sort)
{
	if(sort!=Sorts::None){
		m_pSheet->ResetColumnSort();
	}
	if(m_sort!=sort){
		m_sort=sort;
		OnPropertyChanged(L"sort");
	}
}

void CColumn::OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
{
	if (!_tcsicmp(L"value", name) /*|| !_tcsicmp(L"size", name)*/) {
		if (GetSizingType() == SizingType::Depend || GetSizingType() == SizingType::Fit ||
			(GetSizingType() == SizingType::None && (pCell->GetColSizingType() == SizingType::Depend || pCell->GetColSizingType() == SizingType::Fit))) {
			m_bMeasureValid = false;
		} else {
			//Do nothing, Cell value change 
		}
	}
}

void CColumn::OnPropertyChanged(const wchar_t* name)
{
	m_pSheet->OnColumnPropertyChanged(this, name);
}

//CColumn::int CColumn::GetIndex<VisTag>()const
//{
//	return m_pSheet->Pointer2Index<ColTag, VisTag>(this);
//}
//
//CColumn::int CColumn::GetIndex<AllTag>()const
//{
//	return m_pSheet->Pointer2Index<ColTag, AllTag>(this);
//}

//bool CColumn::SortPredicate(const CCell& lhs, const CCell& rhs)
//{
//
//}

/**
 *  CParentColumn
 */

CParentColumn::CParentColumn(CGridView* pGrid)
	:CColumn(pGrid){} 


FLOAT CParentColumn::Offset()const
{
	FLOAT offset(0);//m_pGrid->GetRect().left is not necessary
	//if(m_pSheet->Visible() && GetIndex<AllTag>()>=0){
	if(m_pSheet->Visible() && GetIndex<VisTag>()>=0){
		offset-=static_cast<CGridView*>(m_pSheet)->GetHorizontalScrollPos();
	}
	return offset;
}

/**
 *  CChildColumn
 */
CChildColumn::CChildColumn(CSheetCell* pSheetCell)
	:CColumn(pSheetCell){} 

FLOAT CChildColumn::Offset()const
{
	return static_cast<CSheetCell*>(m_pSheet)->GetLeft()
			+ (FLOAT)(static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->Line->Width*0.5)
			+ (FLOAT)(static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->Line->Width*0.5)
			+ static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->Padding->left;
}