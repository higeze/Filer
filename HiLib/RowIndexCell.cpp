#include "RowIndexCell.h"
#include "Sheet.h"
#include "Row.h"
#include "Column.h"
#include "MyMenu.h"

std::wstring CRowIndexCell::GetString()
{
	return std::to_wstring(m_pRow->GetIndex<VisTag>() + 1 - m_pSheet->GetFrozenCount<RowTag>());
}



