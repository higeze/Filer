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
//TODO store ptrs
	d2dw::SolidLine line(0.0, 0.0, 0.0, 1.0, 1.0);
	rcPaint.right=rcPaint.left+rcPaint.Height();
	direct.DrawSolidRectangle(line, rcPaint);
	if (m_bool) {
		CComPtr<ID2D1PathGeometry> pPathGeo;
		direct.GetD2D1Factory()->CreatePathGeometry(&pPathGeo);
		CComPtr<ID2D1GeometrySink> pGeoSink;
		pPathGeo->Open(&pGeoSink);
		d2dw::CPointF lt = rcPaint.LeftTop();
		std::array<d2dw::CPointF, 6> pts;
		pts[0].SetPoint(lt.x + 2, lt.y + 6);
		pts[1].SetPoint(lt.x + 2, lt.y + 8);
		pts[2].SetPoint(lt.x + 6, lt.y + 12);
		pts[3].SetPoint(lt.x + 11, lt.y + 4);
		pts[4].SetPoint(lt.x + 11, lt.y + 1);
		pts[5].SetPoint(lt.x + 6, lt.y + 8);
		pGeoSink->BeginFigure(pts[0], D2D1_FIGURE_BEGIN_FILLED);
		for (size_t i = 1; i < pts.size(); i++) {
			pGeoSink->AddLine(pts[i]);
		}
		pGeoSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		pGeoSink->Close();
		d2dw::SolidFill fill(0.0, 0.0, 0.0, 1.0);
		direct.FillSolidGeometry(fill, pPathGeo);
	}


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
