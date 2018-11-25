#include "stdafx.h"
#include "PathCell.h"
#include "FilerGridView.h"
#include "Row.h"
#include "Column.h"
#include "MySize.h"
#include "MyDC.h"


CPathCell::string_type CPathCell::GetString()
{
	return static_cast<CFilerGridView*>(m_pSheet)->GetPath();
}

CRect CPathCell::GetRect()const
{
	return CRect(
		m_pSheet->ZeroPointer<ColTag, VisTag>()->GetLeft(),
		m_pRow->GetTop(),
		m_pSheet->LastPointer<ColTag, VisTag>()->GetRight(),
		m_pRow->GetBottom());
}

CSize CPathCell::GetFitSize(CDC* pDC)
{
	CCell::GetFitSize(pDC);
	m_fitSize.cx = 0;//Zero Width
	return m_fitSize;
}

CSize CPathCell::GetActSize(CDC* pDC)
{
	if(!m_bActMeasureValid){
		auto width = m_pSheet->LastPointer<ColTag, VisTag>()->GetRight() - m_pSheet->ZeroPointer<ColTag, VisTag>()->GetLeft();
		auto fitSize = MeasureSize(pDC); (pDC);
		if(fitSize.cx <= width){
			m_actSize.cx = width;
			m_actSize.cy = fitSize.cy;
			m_bActMeasureValid = true;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(pDC);
			m_bActMeasureValid = true;
		}
	}
	return m_actSize;
}

CSize CPathCell::MeasureContentSizeWithFixedWidth(CDC* pDC)
{
	//Calc Content Rect
	CRect rcCenter(0,0,
		m_pSheet->LastPointer<ColTag, VisTag>()->GetRight() - m_pSheet->ZeroPointer<ColTag, VisTag>()->GetLeft(),0);
	CRect rcContent(InnerBorder2Content(CenterBorder2InnerBorder(rcCenter)));
	//Calc Content Rect
	HFONT hFont=(HFONT)pDC->SelectFont(*m_spProperty->GetFontPtr());
	rcContent.SetRect(0,0,rcContent.Width(),0);
	std::basic_string<TCHAR> str=GetString();
	if(str.empty()){str=_T("a");}
	pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()),str.size(),rcContent,
		DT_CALCRECT|GetFormat(),NULL);
	pDC->SelectFont(hFont);
	//rcContent.right = rcContent.left;//Zero width
	return rcContent.Size();
}

void CPathCell::OnPaint(const PaintEvent& e)
{
	CCell::OnPaint(e);
}

void CPathCell::SetStringCore(const string_type& str)
{
	static_cast<CFilerGridView*>(m_pSheet)->SetPath(str);
}