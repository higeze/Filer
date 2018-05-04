#pragma once
#include "ParentMapColumn.h"

/**
 *  CParentRowHeaderColumn
 *  Most Left side column which show Row number and act as Row header
 */
class CParentRowHeaderColumn:public CParentMapColumn
{
public:
	/**
	 *  Constructor
	 */
	CParentRowHeaderColumn(CGridView* pGrid = nullptr):CParentMapColumn(pGrid){}
	/**
	 *  Destructor
	 */
	virtual ~CParentRowHeaderColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CParentRowHeaderColumn* CloneRaw()const{return new CParentRowHeaderColumn(*this);}
	std::shared_ptr<CParentRowHeaderColumn> Clone()const{return std::shared_ptr<CParentRowHeaderColumn>(CloneRaw());}
	/**
	 *  Name Header Cell Template
	 */
	virtual cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	/**
	 *  Header Cell Template
	 */
	virtual cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	/**
	 *  Header Header Cell Template
	 */
	virtual cell_type HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	/**
	 *  Filter Cell Template
	 */
	virtual cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	/**
	 *  Cell Template
	 */
	virtual cell_type CellTemplate(CRow* pRow, CColumn* pColumn);
};
