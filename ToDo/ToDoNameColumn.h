#pragma once
#include "Column.h"
#include "ParentMapColumn.h"

class CGridView;

class CToDoNameColumn: public CParentDefaultMapColumn
{
	CToDoNameColumn(CGridView* pGrid);
	virtual ~CToDoNameColumn(void) {};
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CToDoNameColumn* CloneRaw()const { return new CToDoNameColumn(*this); }
	//std::shared_ptr<CToDoNameColumn> Clone()const { return std::shared_ptr<CToDoNameColumn>(CloneRaw()); }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;

};

