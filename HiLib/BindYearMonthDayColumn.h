#pragma once
#include "YearMonthDay.h"
#include "FilterCell.h"
#include "MapColumn.h"
#include "HeaderSortCell.h"
#include "GridView.h"
#include "named_arguments.h"

template<typename T> class CBindYearMonthDayCell;

template<typename T>
class CBindYearMonthDayColumn: public CMapColumn
{
	using value_type = T;
	//using bind_cell = CBindYearMonthDayCell<T>;
private:
	std::wstring m_header;
	std::function<reactive_property_ptr<CYearMonthDay>&(T&)> m_path;
public:
	template<typename... Args>
	CBindYearMonthDayColumn(
		CGridView* pSheet,		
		const std::wstring& header,
		std::function<reactive_property_ptr<CYearMonthDay>&(T&)> path,
		Args... args)
		:CMapColumn(pSheet, args...),m_header(header), m_path(path)
	{}

	virtual ~CBindYearMonthDayColumn(void) = default;

	reactive_property_ptr<CYearMonthDay>& GetProperty(T& data) const { return this->m_path(data); }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCell>(this->m_pGrid, pRow, pColumn);
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CHeaderSortCell>(this->m_pGrid, pRow, pColumn, arg<"text"_s>() = this->m_header);
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFilterCell>(this->m_pGrid, pRow, pColumn);
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindYearMonthDayCell<T>>(
			this->m_pGrid, pRow, pColumn);
	}
};

