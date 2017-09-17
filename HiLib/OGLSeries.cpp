#include "OGLSeries.h"
#include "OGLChartWnd.h"
#include "OGLChart.h"

void COGLSeries::OnPaint(OGLPaintEventArgs& e)
{
	if(!m_bVisible)return;

	auto pChart = static_cast<COGLChart*>(m_pParent);
	auto spPen = m_spSeriesProperty->GetPenPtr();
	auto index = static_cast<COGLChart*>(m_pParent)->GetSeriesIndex(this);
	::glLoadName(index);
	::glPushAttrib( GL_CURRENT_BIT | GL_LINE_BIT );
	::glLineWidth( spPen->GetWidth() );

	for(auto row=0;row<(int)m_mVertex.size();++row){
		::glBegin( GL_LINE_STRIP );
		::glColor3ub( spPen->GetColor().GetR(), spPen->GetColor().GetG(), spPen->GetColor().GetB() );
		for(auto col=0;col<(int)m_mVertex[0].size();++col){
			::glVertex3fv((float*)&(pChart->NormalizeVertex(m_mVertex[row][col])));
		}
		::glEnd();
	}

	for(auto col=0;col<(int)m_mVertex[0].size();++col){
		::glBegin( GL_LINE_STRIP );
		::glColor3ub(spPen->GetColor().GetR(),spPen->GetColor().GetG(), spPen->GetColor().GetB() );
		for(auto row=0;row<(int)m_mVertex.size();++row){
			::glVertex3fv((float*)&(pChart->NormalizeVertex(m_mVertex[row][col])));
		}
		::glEnd();
	}

	::glPopAttrib();
	::glLoadName(0);
}

void COGLSeries::SetState(const UIElementState::Type& state)
{

	if(GetState()!=UIElementState::Hot && state == UIElementState::Hot){
		auto spPen = m_spSeriesProperty->GetPenPtr();
		m_spSeriesProperty->SetPenPtr(std::make_shared<CPen>(spPen->GetPenStyle(), spPen->GetWidth()*2, spPen->GetColor()));
	}else if(GetState()==UIElementState::Hot && state != UIElementState::Hot){
		auto spPen = m_spSeriesProperty->GetPenPtr();
		m_spSeriesProperty->SetPenPtr(std::make_shared<CPen>(spPen->GetPenStyle(), spPen->GetWidth()/2, spPen->GetColor()));
	}

	CUIElement::SetState(state);
}
