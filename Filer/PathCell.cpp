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
	return m_pColumn->GetIndex<AllTag>() == 0;
}

std::wstring CPathCell::GetString()
{
	if (IsLeftestCell()) {
		return static_cast<CFilerGridView*>(m_pSheet)->GetPath();
	} else {
		return std::wstring();
	}
}

d2dw::CRectF CPathCell::GetRect()const
{
	return d2dw::CRectF(
		m_pSheet->CellFirstPointer<ColTag, VisTag>()->GetLeft(),
		m_pRow->GetTop(),
		m_pSheet->LastPointer<ColTag, VisTag>()->GetRight(),
		m_pRow->GetBottom());
}

d2dw::CSizeF CPathCell::GetFitSize(d2dw::CDirect2DWrite* pDirect)
{
	CCell::GetFitSize(pDirect);
	m_fitSize.width = 0;//Zero Width
	return m_fitSize;
}

d2dw::CSizeF CPathCell::GetActSize(d2dw::CDirect2DWrite* pDirect)
{
	if(!m_bActMeasureValid){
		auto width = m_pSheet->LastPointer<ColTag, VisTag>()->GetRight() - m_pSheet->CellFirstPointer<ColTag, VisTag>()->GetLeft();
		auto fitSize = MeasureSize(pDirect);
		if(fitSize.width <= width){
			m_actSize.width = width;
			m_actSize.height = fitSize.height;
			m_bActMeasureValid = true;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(pDirect);
			m_bActMeasureValid = true;
		}
	}
	return m_actSize;
}

d2dw::CSizeF CPathCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	d2dw::CRectF rcCenter(0,0,
		m_pSheet->LastPointer<ColTag, VisTag>()->GetRight() - m_pSheet->CellFirstPointer<ColTag, VisTag>()->GetLeft(),0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));
	//Calc Content Rect
	std::basic_string<TCHAR> str=GetString();
	if(str.empty()){str=_T("a");}
	return pDirect->CalcTextSizeWithFixedWidth(*(m_spProperty->Format), str, rcContent.Width());
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