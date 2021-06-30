#pragma once
#include "Column.h"

class CMapColumn:public CColumn
{
protected:
	//std::unordered_map<const CRow*,std::shared_ptr<CCell>> m_mapCell; /**<Map to hold row-cell*/
public:
	template<typename... Args>
	CMapColumn(CSheet* pSheet = nullptr, Args... args):CColumn(pSheet, args...){}
	virtual ~CMapColumn(){}
	CMapColumn(const CMapColumn& column):CColumn(column)
	{
		//Do not copy m_mapCell
	}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CColumn::ShallowCopy(column);
		auto c = dynamic_cast<const CMapColumn&>(column);
		//m_mapCell = c.m_mapCell;

		return *this;
	}
	virtual CMapColumn* CloneRaw()const{return new CMapColumn(*this);}
	std::shared_ptr<CMapColumn> Clone()const{return std::shared_ptr<CMapColumn>(CloneRaw());}
	//virtual std::shared_ptr<CCell>& Cell(CRow* pRow )override;
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;

	//virtual void Clear();// { m_mapCell.clear(); }
	//virtual void Erase(const CRow* pRow);
	//{
	//	if (m_mapCell.find(pRow) != m_mapCell.end()) {
	//		m_mapCell.erase(pRow);
	//	}
	//}
};