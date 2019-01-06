#include "ParentColumnHeaderCell.h"
#include "Sheet.h"
#include "Column.h"
#include "Row.h"
#include "MyRect.h"
#include "MySize.h"
#include "MyDC.h"
#include "CellProperty.h"
#include "MyMenu.h"

CMenu CParentColumnHeaderCell::ContextMenu;
CMenu CParentDefaultColumnHeaderIndexCell::ContextMenu;

std::wstring CParentColumnHeaderCell::GetString()
{
	return std::wstring();
}

std::wstring CParentColumnHeaderCell::GetSortString()
{
	return std::wstring();
}

bool CParentColumnHeaderCell::Filter(const std::wstring& strFilter)const//Not filtered
{
	return true;
}

std::wstring CParentColumnHeaderIndexCell::GetString()
{
	if(m_pColumn->GetVisible()){
		return boost::lexical_cast<std::wstring>(m_pColumn->GetIndex<VisTag>()+1);
	}else{
		return std::wstring(L"0");
	}
}


