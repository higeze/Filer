#pragma once
#include "MapColumn.h"
#include "CheckBoxState.h"
#include "named_arguments.h"
#include "reactive_property.h"
#include "any_tuple.h"

class CBindCheckBoxColumn : public CMapColumn
{
private:
	std::wstring m_header;

	std::function<reactive_property_ptr<CheckBoxState>&(any_tuple&)> m_path;

public:
	CBindCheckBoxColumn(
		CSheet* pSheet,
		const std::wstring& header,
		std::function<reactive_property_ptr<CheckBoxState>&(any_tuple&)> path)
		:CMapColumn(pSheet), m_header(header), m_path(path){}

	virtual ~CBindCheckBoxColumn(void) = default;

	reactive_property_ptr<CheckBoxState>& GetProperty(any_tuple& data) const { return m_path(data); }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

