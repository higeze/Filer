#pragma once
#include "ParentColumnNameHeaderCell.h"
#include "CheckBoxFilterCell.h"
#include "BindCheckBoxCell.h"
#include "MapColumn.h"
#include "named_arguments.h"

template<typename TItem>
class CBindCheckBoxColumn : public CMapColumn
{
private:
	std::wstring m_header;

	std::function<CheckBoxState(const TItem&)> m_getFunction;
	std::function<void(TItem&, const CheckBoxState&)> m_setFunction;

public:
	CBindCheckBoxColumn(CSheet* pSheet,
		const std::wstring& header,
		std::function<CheckBoxState(const TItem&)> getter,
		std::function<void(TItem&, const CheckBoxState&)> setter)
		:CMapColumn(pSheet), m_header(header), m_getFunction(getter), m_setFunction(setter){}

	virtual ~CBindCheckBoxColumn(void) = default;

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CBindCheckBoxColumn* CloneRaw()const { return new CBindCheckBoxColumn(*this); }
	//std::shared_ptr<CToDoNameColumn> Clone()const { return std::shared_ptr<CToDoNameColumn>(CloneRaw()); }
	std::function<CheckBoxState(const TItem&)> GetGetter() { return m_getFunction; }
	std::function<void(TItem&, const CheckBoxState&)> GetSetter() { return m_setFunction; }

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
		return std::make_shared<CBindCheckBoxCell<TItem>>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}
};

