#include "CheckBoxCell.h"
#include "MyRect.h"
#include "MyDC.h"
#include "GridView.h"

CCheckBoxCell::CCheckBoxCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
	:CCell(pSheet, pRow, pColumn), m_checkBox(CheckBoxType::ThreeState, CheckBoxState::False), Dummy(std::make_shared<int>(0))
{
	m_checkBox.State.subscribe([this](const CheckBoxState& state) { OnPropertyChanged(L"value"); }, Dummy);
}

std::wstring CCheckBoxCell::GetString()
{ 
	return State2Str(*m_checkBox.State);
}


void CCheckBoxCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	m_checkBox.Arrange(CRectF(rcPaint.LeftTop(), m_checkBox.DesiredSize()));
	m_checkBox.Render(pDirect);
}

CSizeF CCheckBoxCell::MeasureContentSize(CDirect2DWrite* pDirect)
{
	m_checkBox.Measure(CSizeF(16.f, 16.f));
	return m_checkBox.DesiredSize();
}

CSizeF CCheckBoxCell::MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	return MeasureContentSize(pDirect);
}

void CCheckBoxCell::OnLButtonDown(const LButtonDownEvent& e)
{
	m_checkBox.Toggle();
}

void CCheckBoxCell::OnChar(const CharEvent& e)
{
	if (e.Char == VK_SPACE) {
		m_checkBox.Toggle();
	}
}
