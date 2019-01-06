#pragma once
#include "ParentMapColumn.h"

class CCompareColumn:public CParentDefaultMapColumn
{
public:
	CCompareColumn(CGridView* pGrid = nullptr);
	virtual ~CCompareColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CCompareColumn* CloneRaw()const{return new CCompareColumn(*this);}
	std::shared_ptr<CCompareColumn> Clone()const{return std::shared_ptr<CCompareColumn>(CloneRaw());}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};