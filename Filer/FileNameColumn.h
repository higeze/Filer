#pragma once

#include "ParentMapColumn.h"

class CFileNameColumn: public CParentDefaultMapColumn
{
public:
	CFileNameColumn(CGridView* pGrid = nullptr);
	virtual ~CFileNameColumn(void){};
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileNameColumn* CloneRaw()const{return new CFileNameColumn(*this);}
	std::shared_ptr<CFileNameColumn> Clone()const{return std::shared_ptr<CFileNameColumn>(CloneRaw());}

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

