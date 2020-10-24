#include "CheckBoxCell.h"
#include "MyRect.h"
#include "MyDC.h"
#include "Sheet.h"

CCheckBoxCell::CCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty){}

void CCheckBoxCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	//TODO store ptrs
	SolidLine line(0.0, 0.0, 0.0, 1.0, 1.0);
	rcPaint.right = rcPaint.left + 16.f;
	rcPaint.bottom = rcPaint.top + 16.f;
	pDirect->DrawSolidRectangle(line, rcPaint);
	switch(GetCheckBoxState()){
		case CheckBoxState::True:
		{
			CComPtr<ID2D1PathGeometry> pPathGeo;
			pDirect->GetD2DFactory()->CreatePathGeometry(&pPathGeo);
			CComPtr<ID2D1GeometrySink> pGeoSink;
			pPathGeo->Open(&pGeoSink);
			CPointF lt = rcPaint.LeftTop();
			std::array<CPointF, 6> pts;
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
			SolidFill fill(0.0, 0.0, 0.0, 1.0);
			pDirect->FillSolidGeometry(fill, pPathGeo);
		}
		break;
		case CheckBoxState::Intermediate:
		{
			SolidFill fill(0.0, 0.0, 0.0, 1.0);
			CRectF rcFill(rcPaint);
			rcFill.DeflateRect(4.f);
			pDirect->FillSolidRectangle(fill, rcFill);
		}
		break;
		default:
			break;

	}
}

CSizeF CCheckBoxCell::MeasureContentSize(CDirect2DWrite* pDirect)
{
	return CSizeF(16.f, 16.f);
}

CSizeF CCheckBoxCell::MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	return MeasureContentSize(pDirect);
}

CheckBoxState CCheckBoxCell::Str2State(const std::wstring& str)
{
	if (str == L"1") {
		return CheckBoxState::True;
	} else if (str == L"0") {
		return CheckBoxState::False;
	} else {
		return CheckBoxState::Intermediate;
	}
}

std::wstring CCheckBoxCell::State2Str(const CheckBoxState& state)
{
	switch (state) {
	case CheckBoxState::True:
		return L"1";
	case CheckBoxState::False:
		return L"0";
	case CheckBoxState::Intermediate:
	default:
		return L"";
	}
}

std::wstring CCheckBoxCell::GetString()
{
	return State2Str(GetCheckBoxState());
}

void CCheckBoxCell::SetStringCore(const std::wstring& str)
{
	SetCheckBoxState(Str2State(str));
}

void CCheckBoxCell::OnLButtonDown(const LButtonDownEvent& e)
{
	switch (GetCheckBoxType()) {
		case CheckBoxType::Normal:
			switch (GetCheckBoxState()) {
			case CheckBoxState::True:
				return SetString(L"0");
			case CheckBoxState::False:
			default:
				return SetString(L"1");
			}
		case CheckBoxType::ThreeState:
			switch (GetCheckBoxState()) {
			case CheckBoxState::True:
				return SetString(L"");
			case CheckBoxState::False:
				return SetString(L"1");
			case CheckBoxState::Intermediate:
			default:
				return SetString(L"0");
			}
	}
}
