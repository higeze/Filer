#pragma once
#include "SortCell.h"
#include "CheckBoxFilterCell.h"
#include "BindCheckBoxCell.h"
#include "MapColumn.h"
#include "named_arguments.h"

template<typename... TItems>
class CBindCheckBoxColumn : public CMapColumn
{
private:
	std::wstring m_header;

	std::function<CheckBoxState(const std::tuple<TItems...>&)> m_getFunction;
	std::function<void(std::tuple<TItems...>&, const CheckBoxState&)> m_setFunction;

public:
	CBindCheckBoxColumn(CSheet* pSheet,
		const std::wstring& header,
		std::function<CheckBoxState(const std::tuple<TItems...>&)> getter,
		std::function<void(std::tuple<TItems...>&, const CheckBoxState&)> setter)
		:CMapColumn(pSheet), m_header(header), m_getFunction(getter), m_setFunction(setter){}

	virtual ~CBindCheckBoxColumn(void) = default;

	std::function<CheckBoxState(const std::tuple<TItems...>&)> GetGetter() const { return m_getFunction; }
	std::function<void(std::tuple<TItems...>&, const CheckBoxState&)> GetSetter() const { return m_setFunction; }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = m_header);
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCheckBoxFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindCheckBoxCell<TItems...>>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}
};

