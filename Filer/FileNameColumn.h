#pragma once

#include "MapColumn.h"

class CFileNameColumn: public CMapColumn
{
public:
	template<typename... Args>
	CFileNameColumn(CSheet* pSheet = nullptr, Args... args)
		:CMapColumn(pSheet, args...)
	{
		m_minLength = ::get(arg<"minwidth"_s>(), args..., default_(30.f));
	}
	virtual ~CFileNameColumn(void){};
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileNameColumn* CloneRaw()const{return new CFileNameColumn(*this);}
	std::shared_ptr<CFileNameColumn> Clone()const{return std::shared_ptr<CFileNameColumn>(CloneRaw());}

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

