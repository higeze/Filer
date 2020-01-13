#include "ParentRowHeaderCell.h"
#include "Sheet.h"
#include "Row.h"
#include "Column.h"
#include "MyMenu.h"

CMenu CParentRowHeaderCell::ContextMenu;

std::wstring CParentRowHeaderCell::GetString()
{
	return std::wstring();
}

std::wstring CParentRowHeaderCell::GetSortString()
{
	return std::wstring();
}

bool CParentRowHeaderCell::Filter(const std::wstring& strFilter)const//Not filtered
{
	return true;
}

std::wstring CParentRowHeaderIndexCell::GetString()
{
	return std::to_wstring(m_pRow->GetIndex<VisTag>() + 1 - m_pSheet->GetFrozenCount<RowTag>());
}



