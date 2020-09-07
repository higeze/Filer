#include "FontCell.h"
#include "MyFont.h"
#include "MyDC.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "MySize.h"
#include "Row.h"
#include "Column.h"
#include "GridView.h"

CFontCell::CFontCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,d2dw::CFontF font)
	:CTextCell(pSheet,pRow,pColumn,spProperty),
	m_font(font){}

d2dw::CFontF CFontCell::GetFont(){return m_font;}

void CFontCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	d2dw::FormatF fac(m_font, d2dw::CColorF(0.0f, 0.0f, 0.0f, 1.0f), d2dw::CAlignmentF());
	pDirect->DrawTextLayout(fac, GetString(), rcPaint);
}

void CFontCell::OnLButtonClk(MouseEvent& e)
{
	CHOOSEFONT cf={0};
	LOGFONT logFont={0};
	CFont gdifont = m_font.GetGDIFont();
	memcpy(&logFont,&gdifont.GetLogFont(),sizeof(LOGFONT));
	cf.lStructSize=sizeof(CHOOSEFONT);
	cf.hwndOwner=m_pSheet->GetWndPtr()->m_hWnd;
	cf.lpLogFont=&logFont;
	cf.Flags=CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	if(!ChooseFont(&cf))return;
	if(memcmp(&gdifont.GetLogFont(),&logFont,sizeof(LOGFONT))!=0){
		//TODO bold italic
		m_font.FamilyName = logFont.lfFaceName;
		m_font.Size = (float)logFont.lfHeight;
		OnPropertyChanged(L"value");
	}
}

std::wstring CFontCell::GetString()
{
	std::wstring wstr;
	wstr.append(m_font.FamilyName);
	wstr.append(L", ");
	wstr.append(boost::lexical_cast<std::wstring>((int)(m_pSheet->GetWndPtr()->GetDirectPtr()->Dips2Points(m_font.Size))));
	return wstr;
}
