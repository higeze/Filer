#include "ColorCell.h"
#include "MyColor.h"
#include "MyBrush.h"
#include "MyDC.h"
#include "GridView.h"
#include "GridView.h"
#include "D2DWWindow.h"

CColorCell::CColorCell(CGridView* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,CColorF color)
	:CTextCell(pSheet,pRow,pColumn,spProperty),
	m_color(color){}

CColorF CColorCell::GetColor(){return m_color;}

void CColorCell::PaintBackground(CDirect2DWrite* pDirect,CRectF rcPaint)
{
	SolidFill colorFill(m_color);
	pDirect->FillSolidRectangle(colorFill, rcPaint);
}

void CColorCell::OnLButtonClk(const LButtonClkEvent& e)
{
	//TODO use Custom Choose Color
	COLORREF CustColors[16];
	CHOOSECOLOR cc={0};
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.hwndOwner=m_pGrid->GetWndPtr()->m_hWnd;
	COLORREF rgb = RGB(m_color.r * 255, m_color.g * 255, m_color.b * 255);
	cc.rgbResult=rgb;
	cc.lpCustColors=CustColors;
	cc.Flags=CC_FULLOPEN|CC_RGBINIT;
	if(ChooseColor(&cc)){
		if(rgb!=cc.rgbResult){
			m_color.r = GetRValue(cc.rgbResult)/255.0f;
			m_color.g = GetGValue(cc.rgbResult)/255.0f;
			m_color.b = GetBValue(cc.rgbResult)/255.0f;
			OnPropertyChanged(L"value");
		}
	}
}

std::wstring CColorCell::GetString()
{
	std::wstring wstr(L"RGB:");
	wstr.append(boost::lexical_cast<std::wstring>((int)(m_color.r * 255)));
	wstr.append(L", ");
	wstr.append(boost::lexical_cast<std::wstring>((int)(m_color.g * 255)));
	wstr.append(L", ");
	wstr.append(boost::lexical_cast<std::wstring>((int)(m_color.b * 255)));
	return wstr;
}
