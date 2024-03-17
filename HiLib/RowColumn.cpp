#include "RowColumn.h"
#include "Row.h"
#include "Column.h"
#include "GridView.h"

bool CRowColumn::operator<(const CRowColumn& rhs)const
{
	if(m_pRow!=rhs.m_pRow){
		return m_pRow->GetIndex<VisTag>()<rhs.m_pRow->GetIndex<VisTag>();
	}else{
		return m_pColumn->GetIndex<VisTag>()<rhs.m_pColumn->GetIndex<VisTag>();
	}
}