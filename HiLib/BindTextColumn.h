#pragma once
#include "FilterCell.h"
#include "MapColumn.h"
#include "SortCell.h"
#include "named_arguments.h"

template<typename T>
class CBindTextColumn: public CMapColumn
{
private:
	std::wstring m_header;

	std::function<std::wstring(const T&)> m_getFunction;
	std::function<void(T&, const std::wstring&)> m_setFunction;
	EditMode m_cellEditMode = EditMode::LButtonDownEdit;


public:
	template<typename... Args>
	CBindTextColumn(CSheet* pSheet,
		const std::wstring& header,
		std::function<std::wstring(const T&)> getter,
		std::function<void(T&, const std::wstring&)> setter,
		Args... args)
		:CMapColumn(pSheet, args...), m_header(header), m_getFunction(getter), m_setFunction(setter)
	{
		m_cellEditMode = ::get(arg<"celleditmode"_s>(), args..., default_(EditMode::ReadOnly));
	}

	virtual ~CBindTextColumn(void) = default;

	std::function<std::wstring(const T&)> GetGetter() const { return m_getFunction; }
	std::function<void(T&, const std::wstring&)> GetSetter() const { return m_setFunction; }

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
		return std::make_shared<CBindTextCell<T>>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty(),
			arg<"editmode"_s>() = m_cellEditMode);
	}
};

