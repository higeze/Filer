#pragma once
#include "BindGridView.h"
#include "BindCheckBoxCell.h"
#include "CheckBoxFilterCell.h"
#include "ParentMapColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "named_arguments.h"

template<typename T>
class CBindCheckBoxColumn : public CParentDefaultMapColumn
{
private:
	std::wstring m_header;

	std::function<bool(const T&)> m_getFunction;
	std::function<void(T&, const bool)> m_setFunction;

public:
	CBindCheckBoxColumn(CBindGridView<T>* pGrid,
		const std::wstring& header,
		std::function<bool(const T&)> getter,
		std::function<void(T&, const bool)> setter)
		:CParentDefaultMapColumn(pGrid), m_header(header), m_getFunction(getter), m_setFunction(setter){}

	virtual ~CBindCheckBoxColumn(void) = default;

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CBindCheckBoxColumn* CloneRaw()const { return new CBindCheckBoxColumn(*this); }
	//std::shared_ptr<CToDoNameColumn> Clone()const { return std::shared_ptr<CToDoNameColumn>(CloneRaw()); }
	std::function<bool(const T&)> GetGetter() { return m_getFunction; }
	std::function<void(T&, const bool&)> GetSetter() { return m_setFunction; }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CParentHeaderCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), m_header);
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCheckBoxFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindCheckBoxCell<T>>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}
};

