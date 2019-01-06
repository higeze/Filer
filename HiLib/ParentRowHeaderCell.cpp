#include "ParentRowHeaderCell.h"
#include "Sheet.h"
#include "Row.h"
#include "Column.h"
#include "MyRect.h"
#include "MySize.h"
#include "MyDC.h"
#include "CellProperty.h"
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
	return boost::lexical_cast<std::wstring>(m_pRow->GetIndex<VisTag>()+1);
}



