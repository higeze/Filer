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
		auto fitSize = MeasureSize(pDC);//Call Non-zero width GetFitSize
		if(fitSize.cx <= width){
			m_bActMeasureValid = true;
			m_actSize.cx = width;
			m_actSize.cy = m_fitSize.cy;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(pDC);
			m_fitSize.cx = 0;//Zero Width
		}
	}
	return m_actSize;
}

//CSize CPathCell::MeasureContentSize(CDC* pDC)
//{
//	//Calc Content Rect
//	HFONT hFont=(HFONT)pDC->SelectFont(*m_spProperty->GetFontPtr());
//	CRect rcContent;
//	std::basic_string<TCHAR> str;
//	if(str.empty()){str=_T("a");}
//	pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()),str.size(),rcContent,
//		DT_CALCRECT|GetFormat()&~DT_WORDBREAK,NULL);
//	pDC->SelectFont(hFont);
//	//rcContent.right = rcContent.left;//Zero width
//	return rcContent.Size();
//}

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

//void CPathCell::SetString(const string_type& str)
//{
//	//Filter cell undo redo is set when Post WM_FILTER
//	if(GetString()!=str){
//		string_type newString = str;
//		boost::asio::deadline_timer* pTimer = static_cast<CGridView*>(m_pSheet)->GetTimerPtr();
//		pTimer->expires_from_now(boost::posix_time::milliseconds(500));
//		CCell* pCell = this;
//		HWND hWnd = m_pSheet->GetGridPtr()->m_hWnd;
//		pTimer->async_wait([hWnd,pCell,newString](const boost::system::error_code& error)->void{
//
//			if(error == boost::asio::error::operation_aborted){
//				::OutputDebugStringA("timer canceled\r\n");
//			}else{
//				::OutputDebugStringA("timer filter\r\n");
//				pCell->CCell::SetString(newString);	
//				::PostMessage(hWnd,WM_FILTER,NULL,NULL);
//			}
//		});
//	}
//}

void CPathCell::OnPaint(const PaintEvent& e)
{
	//Paint only when VisibleIndex == 0
	if(m_pColumn->GetIndex<VisTag>() == 0){
		CCell::OnPaint(e);
	}
}

void CPathCell::SetStringCore(const string_type& str)
{
	static_cast<CFilerGridView*>(m_pSheet)->SetPath(str);
}

//void CPathCell::PaintContent(CDC* pDC, CRect rcPaint)
//{
//	HFONT hFont=(HFONT)pDC->SelectFont(*m_spProperty->GetFontPtr());
//	string_type str=GetString();
//	if(!str.empty()){
//		pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
//		pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()),-1,rcPaint,GetFormat(),NULL);	
//	}else{
//		str = L"Filter items...";
//		pDC->SetTextColor(RGB(210,210,210));
//		pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()),-1,rcPaint,GetFormat()&~DT_WORDBREAK|DT_END_ELLIPSIS,NULL);	
//	}
//	pDC->SelectFont(hFont);
//}

