#include "ParentDefaultColumn.h"
#include "Sheet.h"

void CParentDefaultColumn::SetVisible(const bool& bVisible, bool notify)
{
	if(GetVisible()!=bVisible){
		*m_spVisible=bVisible;
		m_pSheet->ColumnPropertyChanged(L"visible");
	}
}