#include "CompareCell.h"
#include "Sheet.h"

CCompareCell::string_type CCompareCell::GetString()
{
	switch(m_pSheet->CheckEqualRow(m_pRow, m_pSheet->Zero<ColTag, VisTag>(), m_pSheet->End<ColTag, VisTag, IndexTag>(), [](CCell* pCell, Compares)->void{})){
	case Compares::Same:
		return L"Same";
	case Compares::Diff:
		return L"Diff";
	case Compares::DiffNE:
		return L"Diff NE";
	default:
		return L"";
	}
}