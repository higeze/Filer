#pragma once
#include "FilterCell.h"
#include "MapColumn.h"
#include "SortCell.h"
#include "Sheet.h"
#include "named_arguments.h"
#include "TaskDateCell.h"
#include "Task.h"

class CDateColumn: public CMapColumn
{
private:
	std::wstring m_header;

public:
	template<typename... Args>
	CDateColumn(CSheet* pSheet,
		const std::wstring& header,
		Args... args)
		:CMapColumn(pSheet, args...), m_header(header)
	{}

	virtual ~CDateColumn(void) = default;

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
		return std::make_shared<CDateCell>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty(),
			arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit);
	}
};

