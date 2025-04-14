#include "RowIndexCell.h"
#include "GridView.h"
#include "Row.h"
#include "Column.h"
#include "MyMenu.h"

std::wstring CRowIndexCell::GetString() const 
{
	return std::to_wstring(m_pRow->GetIndex<VisTag>() + 1 - m_pGrid->GetFrozenCount<RowTag>());
}



