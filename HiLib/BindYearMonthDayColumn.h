#pragma once
#include "BindYearMonthDayCell.h"
#include "FilterCell.h"
#include "MapColumn.h"
#include "SortCell.h"
#include "Sheet.h"
#include "named_arguments.h"

template<typename T>
class CBindYearMonthDayColumn: public CMapColumn
{
private:
	std::wstring m_header;
	std::function<reactive_property_ptr<CYearMonthDay>&(T&)> m_path;
public:
	template<typename... Args>
	CBindYearMonthDayColumn(
		CSheet* pSheet,		
		const std::wstring& header,
		std::function<reactive_property_ptr<CYearMonthDay>&(T&)> path,
		Args... args)
		:CMapColumn(pSheet, args...),m_header(header), m_path(path)
	{}

	virtual ~CBindYearMonthDayColumn(void) = default;

	reactive_property_ptr<CYearMonthDay>& GetProperty(T& data) const { return m_path(data); }

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
		return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindYearMonthDayCell<T>>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}
};

