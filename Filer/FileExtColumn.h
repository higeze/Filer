#pragma once

#include "MapColumn.h"

class CFileExtColumn:public CMapColumn
{
public:
	CFileExtColumn(CSheet* pSheet = nullptr)
		:CMapColumn(pSheet){}
	~CFileExtColumn(void){}

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileExtColumn* CloneRaw()const{return new CFileExtColumn(*this);}
	virtual std::shared_ptr<CFileExtColumn> Clone()const{return std::shared_ptr<CFileExtColumn>(CloneRaw());}

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

