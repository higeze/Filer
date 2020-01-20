#pragma once
#include "BindGridView.h"
#include "BindTextCell.h"
#include "ParentMapColumn.h"

template<typename T> class CBindTextCell;
class CCell;

template<typename T>
class CBindTextColumn: public CParentDefaultMapColumn
{
private:
	std::function<std::wstring(const T&)> m_getFunction;
	std::function<void(T&, std::wstring)> m_setFunction;

public:
	CBindTextColumn(CBindGridView* pGrid)
		:CParentDefaultMapColumn(pGrid){}

	virtual ~CBindTextColumn(void) = default;

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CBindTextColumn* CloneRaw()const { return new CBindTextColumn(*this); }
	//std::shared_ptr<CToDoNameColumn> Clone()const { return std::shared_ptr<CToDoNameColumn>(CloneRaw()); }
	std::function<std::wstring(const T&)> GetGettter() { return m_getFunction; }
	std::function<void(T&, std::wstring)> GetSetter() { return m_setFunction; }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CParentHeaderCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), L"Name");
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindTextCell<T>>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}
};

