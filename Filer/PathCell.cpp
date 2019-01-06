#include "stdafx.h"
#include "PathCell.h"
#include "FilerGridView.h"
#include "Row.h"
#include "Column.h"
#include "MySize.h"
#include "MyDC.h"


std::wstring CPathCell::GetString()
{
	return static_cast<CFilerGridView*>(m_pSheet)->GetPath();
}

d2dw::CRectF CPathCell::GetRect()const
{
	return d2dw::CRectF(
		m_pSheet->ZeroPointer<ColTag, VisTag>()->GetLeft(),
		m_pRow->GetTop(),
		m_pSheet->LastPointer<ColTag, VisTag>()->GetRight(),
		m_pRow->GetBottom());
}

d2dw::CSizeF CPathCell::GetFitSize(d2dw::CDirect2DWrite& direct)
{
	CCell::GetFitSize(direct);
	m_fitSize.width = 0;//Zero Width
	return m_fitSize;
}

d2dw::CSizeF CPathCell::GetActSize(d2dw::CDirect2DWrite& direct)
{
	if(!m_bActMeasureValid){
		auto width = m_pSheet->LastPointer<ColTag, VisTag>()->GetRight() - m_pSheet->ZeroPointer<ColTag, VisTag>()->GetLeft();
		auto fitSize = MeasureSize(direct);
		if(fitSize.width <= width){
			m_actSize.width = width;
			m_actSize.height = fitSize.height;
			m_bActMeasureValid = true;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(direct);
			m_bActMeasureValid = true;
		}
	}
	return m_actSize;
}

d2dw::CSizeF CPathCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
	d2dw::CRectF rcCenter(0,0,
		m_pSheet->LastPointer<ColTag, VisTag>()->GetRight() - m_pSheet->ZeroPointer<ColTag, VisTag>()->GetLeft(),0);
	d2dw::CRectF rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));
	//Calc Content Rect
	std::basic_string<TCHAR> str=GetString();
	if(str.empty()){str=_T("a");}
	return direct.CalcTextSizeWithFixedWidth(m_spProperty->FontAndColor->Font, str, rcContent.Width());
}

void CPathCell::OnPaint(const PaintEvent& e)
{
	CCell::OnPaint(e);
}

void CPathCell::SetStringCore(const std::wstring& str)
{
	static_cast<CFilerGridView*>(m_pSheet)->SetPath(str);
}