#include "FontCell.h"
#include "MyFont.h"
#include "MyDC.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "MySize.h"
#include "Row.h"
#include "Column.h"
#include "GridView.h"

CFontCell::CFontCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,d2dw::Font font)
	:CTextCell(pSheet,pRow,pColumn,spProperty),
	m_font(font){}

d2dw::Font CFontCell::GetFont(){return m_font;}

void CFontCell::PaintContent(d2dw::CDirect2DWrite& direct,d2dw::CRectF rcPaint)
{
	//TODOTODO
	//HFONT hFont=(HFONT)pDC->SelectFont(m_font);
	//std::wstring str=GetString();
	//pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()),-1,rcPaint,GetFormat(),NULL);		
	//pDC->SelectFont(hFont);
}

void CFontCell::OnLButtonClk(MouseEvent& e)
{
	//TODOTODO
	//CHOOSEFONT cf={0};
	//LOGFONT logFont={0};
	//memcpy(&logFont,&m_font.GetLogFont(),sizeof(LOGFONT));
	//cf.lStructSize=sizeof(CHOOSEFONT);
	//cf.hwndOwner=m_pSheet->GetGridPtr()->m_hWnd;
	//cf.lpLogFont=&logFont;
	//cf.Flags=CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	//if(!ChooseFont(&cf))return;
	//if(memcmp(&m_font.GetLogFont(),&logFont,sizeof(LOGFONT))!=0){
	//	
	//	m_font=logFont;
	//	OnPropertyChanged(L"value");
	//}
}

std::wstring CFontCell::GetString()
{
	std::wstring wstr;
	wstr.append(m_font.FamilyName);
	wstr.append(L", ");
	wstr.append(boost::lexical_cast<std::wstring>((int)(m_pSheet->GetGridPtr()->GetDirect()->Dips2Points(m_font.Size))));
	return wstr;
}
