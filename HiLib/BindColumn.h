#pragma once
#include "MapColumn.h"
#include "RowIndexColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "BindCell.h"
#include "Sheet.h"


template<typename TItem>
class CBindColumn:public CMapColumn
{
private:
	std::wstring m_header;
	std::function<std::wstring(TItem&)> m_getFunction;
	std::function<void(T&,std::wstring)> m_setFunction;

public:
	CBindColumn():CMapColumn(){}
	CBindColumn(CSheet* pSheet,
		std::wstring header, 
		std::function<std::wstring(T&)> getFunction,
		std::function<void(T&,std::wstring)> setFunction)
		:CMapColumn(pSheet),m_header(header), m_getFunction(getFunction),m_setFunction(setFunction){}
	virtual ~CBindColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		auto c = dynamic_cast<const CBindColumn<T>&>(column);
		m_header = c.m_header;
		m_getFunction = c.m_getFunction;
		m_setFunction = c.m_setFunction;
		return *this;
	}

	virtual CBindColumn* CloneRaw()const{return new CBindColumn<TItem>(*this);}
	std::shared_ptr<CBindColumn> Clone()const{return std::shared_ptr<CBindColumn<TItem>>(CloneRaw());}

	std::function<std::wstring(TItem&)> GetGetFunction(){return m_getFunction;}
	std::function<void(TItem&,std::wstring)> GetSetFunction(){return m_setFunction;}

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
		return std::make_shared<CBindCell<TItem>>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
	}
};

template<typename TItem>
class CBindRowHeaderColumn:public CRowIndexColumn
{
private:
	std::function<bool(TItem&)> m_getCheckedFunction;
public:
	CBindRowHeaderColumn(CSheet* pSheet, std::function<bool(TItem&)> getCheckedFunction)
		:CRowIndexColumn(pSheet),m_getCheckedFunction(getCheckedFunction){}
	virtual ~CBindRowHeaderColumn(){}

	std::function<bool(TItem&)> GetGetCheckedFunction(){return m_getCheckedFunction;}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindRowHeaderCell<TItem>>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
	}
};