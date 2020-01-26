#include "CheckBoxCell.h"
#include "MyRect.h"
#include "MyDC.h"
#include "Sheet.h"

CCheckBoxCell::CCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty){}

void CCheckBoxCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	//TODO store ptrs
	d2dw::SolidLine line(0.0, 0.0, 0.0, 1.0, 1.0);
	rcPaint.right = rcPaint.left + rcPaint.Height();
	pDirect->DrawSolidRectangle(line, rcPaint);
	if (GetCheck()) {
		CComPtr<ID2D1PathGeometry> pPathGeo;
		pDirect->GetD2D1Factory()->CreatePathGeometry(&pPathGeo);
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
		pDirect->FillSolidGeometry(fill, pPathGeo);
	}
}

d2dw::CSizeF CCheckBoxCell::MeasureSize(d2dw::CDirect2DWrite* pDirect)
{
	d2dw::CRectF rcCenter = (InnerBorder2CenterBorder(Content2InnerBorder(d2dw::CRectF(0.f, 0.f, 5.f, 5.f))));
	return rcCenter.Size();
}

d2dw::CSizeF CCheckBoxCell::MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	return MeasureSize(pDirect);
}

std::wstring CCheckBoxCell::GetString()
{
	return boost::lexical_cast<std::wstring>(GetCheck());
}

void CCheckBoxCell::SetStringCore(const std::wstring& str)
{
	SetCheck(boost::lexical_cast<bool>(str));
}

void CCheckBoxCell::OnLButtonDown(const LButtonDownEvent& e)
{
	SetString(boost::lexical_cast<std::wstring>(!GetCheck()));
}

bool CCheckBoxCell::IsComparable()const { return false; }
