//#include "stdafx.h"
#include "PathCell.h"
#include "FilerGridView.h"
#include "Row.h"
#include "Column.h"
#include "MySize.h"
#include "MyDC.h"
#include "CellProperty.h"


bool CPathCell::IsLeftestCell()const
{
	return m_pColumn->GetIndex<VisTag>() == m_pSheet->GetFrozenCount<ColTag>();
}

std::wstring CPathCell::GetString()
{
	//if (IsLeftestCell()) {
		return static_cast<CFilerGridView*>(m_pSheet)->GetPath();
	//} else {
	//	return std::wstring();
	//}
}

void CPathCell::SetString(const std::wstring& str, bool notify)
{
	if (GetString() != str) {
		SetStringCore(str);
	}
}

CRectF CPathCell::GetRectInWnd()const
{
	return CRectF(
		m_pSheet->GetContainer<ColTag, VisTag>()[m_pSheet->GetFrozenCount<ColTag>()]->GetLeft(),
		m_pRow->GetTop(),
		m_pSheet->GetContainer<ColTag, VisTag>().back()->GetRight(),
		m_pRow->GetBottom());
}

CSizeF CPathCell::GetFitSize(CDirect2DWrite* pDirect)
{
	CCell::GetFitSize(pDirect);
	m_fitSize.width = 0;//Zero Width
	return m_fitSize;
}

CSizeF CPathCell::GetActSize(CDirect2DWrite* pDirect)
{
	if(!m_isActMeasureValid){
		auto width = m_pSheet->GetContainer<ColTag, VisTag>().back()->GetRight() - m_pSheet->GetContainer<ColTag, VisTag>()[m_pSheet->GetFrozenCount<ColTag>()]->GetLeft();
		auto fitSize = MeasureSize(pDirect);
		if(fitSize.width <= width){
			m_actSize.width = width;
			m_actSize.height = fitSize.height;
			m_isActMeasureValid = true;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(pDirect);
			m_isActMeasureValid = true;
		}
	}
	return m_actSize;
}

CSizeF CPathCell::MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	CRectF rcCenter(0,0,
		m_pSheet->GetContainer<ColTag, VisTag>().back()->GetRight() - m_pSheet->GetContainer<ColTag, VisTag>()[m_pSheet->GetFrozenCount<ColTag>()]->GetLeft(),0);
	CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));
	//Calc Content Rect
	std::basic_string<TCHAR> str=IsLeftestCell()?GetString():std::basic_string<TCHAR>();
	if(str.empty()){str=_T("a");}
	return pDirect->CalcTextSizeWithFixedWidth(*(m_spCellProperty->Format), str, rcContent.Width());
}

void CPathCell::OnPaint(const PaintEvent& e)
{
	if(IsLeftestCell()){
		CCell::OnPaint(e);
	}
}

void CPathCell::SetStringCore(const std::wstring& str)
{
	static_cast<CFilerGridView*>(m_pSheet)->SetPath(str);
}