#pragma once
#include "MapColumn.h"

/**
 *  CParentRowHeaderColumn
 *  Most Left side column which show Row number and act as Row header
 */
class CParentRowHeaderColumn:public CMapColumn
{
public:
	CParentRowHeaderColumn(CSheet* pSheet = nullptr):CMapColumn(pSheet)
	{
		m_isMaxLengthFit = true;
		m_isMinLengthFit = true;
	}
	virtual ~CParentRowHeaderColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CParentRowHeaderColumn* CloneRaw()const{return new CParentRowHeaderColumn(*this);}
	std::shared_ptr<CParentRowHeaderColumn> Clone()const{return std::shared_ptr<CParentRowHeaderColumn>(CloneRaw());}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);

//	virtual SizingType GetSizingType()const override { return SizingType::AlwaysFit; }
	virtual bool IsTrackable()const override { return true; }

};
