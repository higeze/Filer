#pragma once
#include "SortCell.h"
#include "CheckBoxFilterCell.h"
#include "BindCheckBoxCell.h"
#include "MapColumn.h"
#include "named_arguments.h"

template<typename T>
class CBindCheckBoxColumn : public CMapColumn
{
private:
	std::wstring m_header;

	std::function<reactive_property_ptr<CheckBoxState>&(T&)> m_path;

public:
	CBindCheckBoxColumn(
		CGridView* pSheet,
		const std::wstring& header,
		std::function<reactive_property_ptr<CheckBoxState>&(T&)> path)
		:CMapColumn(pSheet), m_header(header), m_path(path){}

	virtual ~CBindCheckBoxColumn(void) = default;

	reactive_property_ptr<CheckBoxState>& GetProperty(T& data) const { return m_path(data); }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCell>(m_pGrid, pRow, pColumn, m_pGrid->GetCellProperty());
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CSortCell>(m_pGrid, pRow, pColumn, m_pGrid->GetHeaderProperty(), arg<"text"_s>() = m_header);
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCheckBoxFilterCell>(m_pGrid, pRow, pColumn, m_pGrid->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CBindCheckBoxCell<T>>(
			m_pGrid, pRow, pColumn, m_pGrid->GetCellProperty());
	}
};

