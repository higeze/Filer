#include "ParentColumnHeaderHeaderCell.h"
#include "Sheet.h"
#include "Column.h"
#include "Row.h"
#include "MyRect.h"
#include "MySize.h"
#include "MyDC.h"
#include "CellProperty.h"

std::wstring CParentColumnHeaderHeaderCell::GetString()
{
	return std::wstring();
}

std::wstring CParentColumnHeaderHeaderCell::GetSortString()
{
	return std::wstring();
}

bool CParentColumnHeaderHeaderCell::Filter(const std::wstring& strFilter)const//Not filtered
{
	return true;
}


