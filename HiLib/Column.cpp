#include "Column.h"
#include "Sheet.h"
#include "CellProperty.h"

FLOAT CColumn::GetStart() { return m_start/* + m_pSheet->GetRectInWnd().left*/; }

FLOAT CColumn::GetEnd() { return m_start + m_length /* + +m_pSheet->GetRectInWnd().left*/; }

FLOAT CColumn::GetLength()
{
	if (m_isInit) {
		SetLength(m_pSheet->GetColumnInitWidth(this), false);
		m_isInit = false;
		m_isMeasureValid = true;
	}

	if (!m_isMeasureValid) {
		FitLength();
		m_isMeasureValid = true;
	}

	if (!m_isFitMeasureValid) {
		BoundLength();
	}
	return m_length;
}

FLOAT CColumn::GetVirtualLength()
{
	return 30.f;
}


FLOAT CColumn::GetFitLength()
{
	if (!m_isFitMeasureValid) {
		m_fitLength = m_pSheet->GetColumnFitWidth(this);
		if (m_fitLength < 1.f) {
			auto aa = 1;
		}
		m_isFitMeasureValid = true;
	} else {
		if (m_fitLength < 1.f) {
			auto aa = 1;
		}

	}
	return m_fitLength;
}

void CColumn::SetIsVisible(const bool& bVisible, bool notify)
{
	if(*m_spVisible!=bVisible){
		*m_spVisible=bVisible;
		if(notify){
			OnPropertyChanged(L"visible");
		}
	}
}
void CColumn::SetIsSelected(const bool& bSelected)
{
	if(m_bSelected!=bSelected){
		m_bSelected=bSelected;
		OnPropertyChanged(L"selected");
	}
}
void CColumn::SetSort(const Sorts& sort)
{
	if(sort!=Sorts::None){
		m_pSheet->ResetColumnSort();
	}
	if(m_sort!=sort){
		m_sort=sort;
		OnPropertyChanged(L"sort");
	}
}

void CColumn::OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
{
	if (!_tcsicmp(L"value", name) || !_tcsicmp(L"size", name)) {
		if (!pCell->GetIsWrappable()) {
			m_isMeasureValid = false;
		}
		m_isFitMeasureValid = false;
	}
}

void CColumn::OnPropertyChanged(const wchar_t* name)
{
	m_pSheet->OnColumnPropertyChanged(this, name);
}
