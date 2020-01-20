#pragma once
#include "BindGridView.h"
#include "BindTextCell.h"
#include "FilterCell.h"
#include "ParentMapColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "named_arguments.h"

template<typename T> class CBindTextCell;
class CCell;

template<typename T>
class CBindTextColumn: public CParentDefaultMapColumn
{
private:
	std::wstring m_header;

	std::function<std::wstring(const T&)> m_getFunction;
	std::function<void(T&, const std::wstring&)> m_setFunction;

public:
	CBindTextColumn(CBindGridView<T>* pGrid,
		const std::wstring& header,
		std::function<std::wstring(const T&)> getter,
		std::function<void(T&, const std::wstring&)> setter)
		:CParentDefaultMapColumn(pGrid), m_header(header), m_getFunction(getter), m_setFunction(setter){}

	virtual ~CBindTextColumn(void) = default;

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CBindTextColumn* CloneRaw()const { return new CBindTextColumn(*this); }
	//std::shared_ptr<CToDoNameColumn> Clone()const { return std::shared_ptr<CToDoNameColumn>(CloneRaw()); }
	std::function<std::wstring(const T&)> GetGetter() { return m_getFunction; }
	std::function<void(T&, const std::wstring&)> GetSetter() { return m_setFunction; }

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
		return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindTextCell<T>>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty(),
			arg<'e'>() = EditMode::FocusedSingleClickEdit);
	}
};

