#pragma once
#include "FilterCell.h"
#include "MapColumn.h"
#include "named_arguments.h"
#include "any_tuple.h"

class CBindTextColumn: public CMapColumn
{
private:
	std::wstring m_header;

	std::function<std::wstring(const any_tuple&)> m_getFunction;
	std::function<void(any_tuple&, const std::wstring&)> m_setFunction;
	EditMode m_cellEditMode = EditMode::LButtonDownEdit;


public:
	template<typename... Args>
	CBindTextColumn(CSheet* pSheet,
		const std::wstring& header,
		std::function<std::wstring(const any_tuple&)> getter,
		std::function<void(any_tuple&, const std::wstring&)> setter,
		Args... args)
		:CMapColumn(pSheet, args...), m_header(header), m_getFunction(getter), m_setFunction(setter)
	{
		m_cellEditMode = ::get(arg<"celleditmode"_s>(), args..., default_(EditMode::ReadOnly));
	}

	virtual ~CBindTextColumn(void) = default;

	std::function<std::wstring(const any_tuple&)> GetGetter() const { return m_getFunction; }
	std::function<void(any_tuple&, const std::wstring&)> GetSetter() const { return m_setFunction; }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

