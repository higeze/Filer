#pragma once
#include "MapColumn.h"
#include "YearMonthDay.h"
#include "named_arguments.h"
#include "any_tuple.h"

class CBindYearMonthDayColumn: public CMapColumn
{
private:
	std::wstring m_header;
	std::function<reactive_property_ptr<CYearMonthDay>&(any_tuple&)> m_path;
public:
	template<typename... Args>
	CBindYearMonthDayColumn(
		CSheet* pSheet,		
		const std::wstring& header,
		std::function<reactive_property_ptr<CYearMonthDay>&(any_tuple&)> path,
		Args... args)
		:CMapColumn(pSheet, args...),m_header(header), m_path(path)
	{}

	virtual ~CBindYearMonthDayColumn(void) = default;

	reactive_property_ptr<CYearMonthDay>& GetProperty(any_tuple& data) const { return m_path(data); }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

