#include "CheckBox.h"

CCheckBox::CCheckBox(const CheckBoxType& type, const CheckBoxState& state)
	:Type(make_reactive_property<CheckBoxType>(type)), State(make_reactive_property<CheckBoxState>(state)){}

void CCheckBox::Render(CDirect2DWrite* pDirect)
{
	SolidLine line(0.0, 0.0, 0.0, 1.0, 1.0);
	pDirect->DrawSolidRectangleByLine(line, m_rect);
	switch(State->get_const()){
		case CheckBoxState::None:
		{
			pDirect->DrawSolidLine(line, CPointF(m_rect.left + 2, (m_rect.top + m_rect.bottom) / 2), CPointF(m_rect.right - 2, (m_rect.top + m_rect.bottom) / 2));
		}
		break;
		case CheckBoxState::True:
		{
			CComPtr<ID2D1PathGeometry> pPathGeo;
			pDirect->GetD2DFactory()->CreatePathGeometry(&pPathGeo);
			CComPtr<ID2D1GeometrySink> pGeoSink;
			pPathGeo->Open(&pGeoSink);
			CPointF lt = m_rect.LeftTop();
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
			CRectF rcFill(m_rect);
			rcFill.DeflateRect(4.f);
			pDirect->FillSolidRectangle(fill, rcFill);
		}
		break;
		default:
			break;

	}
}

void CCheckBox::Toggle()
{

	switch (Type->get_const()) {
		case CheckBoxType::Normal:
			switch (State->get_const()) {
				case CheckBoxState::False:
					return State->set(CheckBoxState::True);
				case CheckBoxState::True:
				default:
					return State->set(CheckBoxState::False);
			}
		case CheckBoxType::ThreeState:
			//return SetString(std::to_wstring((static_cast<int>(GetCheckBoxState()) + 1) % 4));
			switch (State->get_const()) {
				case CheckBoxState::False:
					return State->set(CheckBoxState::Intermediate);
				case CheckBoxState::Intermediate:
					return State->set(CheckBoxState::True);
				case CheckBoxState::True:
				default:
					return State->set(CheckBoxState::False);
			}
	}
}

const void* CCheckBox::HitTestCore(const CPointF& ptInWnd) const
{
	if (m_rect.PtInRect(ptInWnd)) {
		return reinterpret_cast<const void*>(this);
	} else {
		return nullptr;
	}
}