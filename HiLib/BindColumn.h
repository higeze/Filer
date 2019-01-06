#pragma once
#include "ParentMapColumn.h"
#include "ParentRowHeaderColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "BindCell.h"
#include "Sheet.h"


template<typename T>
class CBindColumn:public CParentDefaultMapColumn
{
private:
	std::wstring m_header;
	std::function<std::wstring(T&)> m_getFunction;
	std::function<void(T&,std::wstring)> m_setFunction;

public:
	CBindColumn():CParentDefaultMapColumn(){}
	CBindColumn(CGridView* pGrid, std::wstring header, std::function<std::wstring(T&)> getFunction, std::function<void(T&,std::wstring)> setFunction)
		:CParentDefaultMapColumn(pGrid),m_header(header), m_getFunction(getFunction),m_setFunction(setFunction){}
	virtual ~CBindColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		auto c = dynamic_cast<const CBindColumn<T>&>(column);
		m_header = c.m_header;
		m_getFunction = c.m_getFunction;
		m_setFunction = c.m_setFunction;
		return *this;
	}

	virtual CBindColumn* CloneRaw()const{return new CBindColumn<T>(*this);}
	std::shared_ptr<CBindColumn> Clone()const{return std::shared_ptr<CBindColumn<T>>(CloneRaw());}

	std::function<std::wstring(T&)> GetGetFunction(){return m_getFunction;}
	std::function<void(T&,std::wstring)> GetSetFunction(){return m_setFunction;}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),m_header);
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindCell<T>>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
	}
};

template<typename T>
class CBindRowHeaderColumn:public CParentRowHeaderColumn
{
private:
	std::function<bool(T&)> m_getCheckedFunction;
public:
	CBindRowHeaderColumn(CGridView* pGrid, std::function<bool(T&)> getCheckedFunction)
		:CParentRowHeaderColumn(pGrid),m_getCheckedFunction(getCheckedFunction){}
	virtual ~CBindRowHeaderColumn(){}

	std::function<bool(T&)> GetGetCheckedFunction(){return m_getCheckedFunction;}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindRowHeaderCell<T>>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
	}
};