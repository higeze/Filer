#pragma once

#include "ParentMapColumn.h"

class CFileExtColumn:public CParentDefaultMapColumn
{
public:
	CFileExtColumn(CGridView* pGrid = nullptr)
		:CParentDefaultMapColumn(pGrid){}
	~CFileExtColumn(void){}

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileExtColumn* CloneRaw()const{return new CFileExtColumn(*this);}
	virtual std::shared_ptr<CFileExtColumn> Clone()const{return std::shared_ptr<CFileExtColumn>(CloneRaw());}

	virtual cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual cell_type CellTemplate(CRow* pRow, CColumn* pColumn);
};

