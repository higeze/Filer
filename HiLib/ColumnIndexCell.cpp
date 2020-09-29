#include "ColumnIndexCell.h"
#include "Column.h"

std::wstring CColumnIndexCell::GetString()
{
	if(m_pColumn->GetIsVisible()){
		return std::to_wstring(m_pColumn->GetIndex<VisTag>()+1);
	}else{
		return std::wstring(L"0");
	}
}


