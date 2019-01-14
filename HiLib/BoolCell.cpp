#include "BoolCell.h"
#include "MyRect.h"
#include "MyDC.h"
#include "Sheet.h"
//

CBoolCell::CBoolCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,bool b)
	:CTextCell(pSheet,pRow,pColumn,spProperty),m_bool(b)//,m_checkBox(NULL,[&]()->bool{return m_bool;},[&](const bool& b)->void{m_bool = b;})
{
}

void CBoolCell::PaintContent(d2dw::CDirect2DWrite& direct,d2dw::CRectF rcPaint)
{
	//TODOTODO
	//rcPaint.right=rcPaint.left+rcPaint.Height();
	//m_checkBox.SetRect(rcPaint);
	//m_checkBox.OnPaint(PaintEvent(pDC));
}
d2dw::CSizeF CBoolCell::MeasureSize(d2dw::CDirect2DWrite& direct)
{
	d2dw::CRectF rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(d2dw::CRectF(0,0,5,5))));
	return rcCenter.Size();	
}

d2dw::CSizeF CBoolCell::MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	return MeasureSize(direct);
}

std::wstring CBoolCell::GetString()
{
	return boost::lexical_cast<std::wstring>(m_bool);
}

void CBoolCell::SetStringCore(const std::wstring& str)
{
	m_bool = boost::lexical_cast<bool>(str);
}

void CBoolCell::OnLButtonDown(const LButtonDownEvent& e)
{
	SetString(boost::lexical_cast<std::wstring>(!(m_bool)));
}

bool CBoolCell::IsComparable()const{return false;}
