#include "FontCell.h"
#include "MyFont.h"
#include "MyDC.h"
#include "CellProperty.h"
#include "GridView.h"
#include "MySize.h"
#include "Row.h"
#include "Column.h"
#include "GridView.h"
#include "D2DWWindow.h"

CFontCell::CFontCell(CGridView* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,CFontF font)
	:CTextCell(pSheet,pRow,pColumn,spProperty),
	m_font(font){}

CFontF CFontCell::GetFont(){return m_font;}

void CFontCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	FormatF fac(m_font, CColorF(0.0f, 0.0f, 0.0f, 1.0f), CAlignmentF());
	pDirect->DrawTextLayout(fac, GetString(), rcPaint);
}

void CFontCell::OnLButtonClk(MouseEvent& e)
{
	CHOOSEFONT cf={0};
	LOGFONT logFont={0};
	LOGFONT srcLogFont = m_font.GetGDIFont().GetLogFont();
	memcpy(reinterpret_cast<void*>(&logFont),reinterpret_cast<const void*>(&srcLogFont),sizeof(LOGFONT));
	cf.lStructSize=sizeof(CHOOSEFONT);
	cf.hwndOwner=m_pGrid->GetWndPtr()->m_hWnd;
	cf.lpLogFont=&logFont;
	cf.Flags=CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	if(!ChooseFont(&cf))return;
	if(memcmp(reinterpret_cast<const void*>(&logFont),reinterpret_cast<const void*>(&srcLogFont),sizeof(LOGFONT))!=0){
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
	wstr.append(boost::lexical_cast<std::wstring>((int)(m_pGrid->GetWndPtr()->GetDirectPtr()->Dips2Points(m_font.Size))));
	return wstr;
}
