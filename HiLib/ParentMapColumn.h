#pragma once
#include "Column.h"

/**
 *  CParentMapColumn
 *  This class hold cell in map<CRow, std::shared_ptr<CCell>>
 */
class CParentMapColumn:public CParentColumn
{
protected:
	std::unordered_map<CRow*,std::shared_ptr<CCell>> m_mapCell; /**<Map to hold row-cell*/
public:
	/**
	 *  Constructor
	 */
	CParentMapColumn(CGridView* pGrid = nullptr):CParentColumn(pGrid){}
	/**
	 *  Destructor
	 */
	virtual ~CParentMapColumn(){}
	CParentMapColumn(const CParentMapColumn& column):CParentColumn(column)
	{
		//Do not copy m_mapCell
	}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentColumn::ShallowCopy(column);
		auto c = dynamic_cast<const CParentMapColumn&>(column);
		m_mapCell = c.m_mapCell;

		return *this;
	}
	virtual CParentMapColumn* CloneRaw()const{return new CParentMapColumn(*this);}
	std::shared_ptr<CParentMapColumn> Clone()const{return std::shared_ptr<CParentMapColumn>(CloneRaw());}
	/**
	 *  Cell accessor
	 */
	virtual std::shared_ptr<CCell>& Cell(CRow* pRow )override;
	/**
	 *  Header Cell Template
	 */
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	/**
	 *  Header Header Cell Template
	 */
	virtual std::shared_ptr<CCell> HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	/**
	 *  Name Header Cell Template
	 */
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	/**
	 *  Filter Cell Template
	 */
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	/**
	 *  Cell Template
	 */
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;

	virtual void Clear(){m_mapCell.clear();}
};

class CParentDefaultMapColumn:public CParentMapColumn
{
public:
	/**
	 *  Constructor
	 */
	CParentDefaultMapColumn(CGridView* pGrid = nullptr):CParentMapColumn(pGrid){}
	/**
	 *  Destructor
	 */
	virtual ~CParentDefaultMapColumn(){}
	CParentDefaultMapColumn(const CParentDefaultMapColumn& column):CParentMapColumn(column)
	{}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CParentDefaultMapColumn* CloneRaw()const{return new CParentDefaultMapColumn(*this);}
	std::shared_ptr<CParentDefaultMapColumn> Clone()const{return std::shared_ptr<CParentDefaultMapColumn>(CloneRaw());}
	/**
	 *  Header Cell Template
	 */
	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;

};
