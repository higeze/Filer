#pragma once
#include "ParentMapColumn.h"

class CFileLastWriteColumn:public CParentDefaultMapColumn
{
public:
	CFileLastWriteColumn(CGridView* pGrid = nullptr);
	~CFileLastWriteColumn(void){}

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileLastWriteColumn* CloneRaw()const{return new CFileLastWriteColumn(*this);}
	virtual std::shared_ptr<CFileLastWriteColumn> Clone()const{return std::shared_ptr<CFileLastWriteColumn>(CloneRaw());}

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

