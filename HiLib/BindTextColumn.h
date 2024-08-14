#pragma once
#include "MapColumn.h"
#include "FilterCell.h"
#include "HeaderSortCell.h"
#include "named_arguments.h"

template<typename T> class CBindTextCell;

template<typename T>
class CBindTextColumn: public CMapColumn
{
protected:
	std::wstring m_header;

	std::function<std::wstring(const T&)> m_getFunction;
	std::function<void(T&, const std::wstring&)> m_setFunction;
	EditMode m_cellEditMode = EditMode::LButtonDownEdit;

public:
	template<typename... Args>
	CBindTextColumn(CGridView* pSheet,
		const std::wstring& header,
		std::function<std::wstring(const T&)> getter,
		std::function<void(T&, const std::wstring&)> setter,
		Args... args)
		:CMapColumn(pSheet, args...), m_header(header), m_getFunction(getter), m_setFunction(setter)
	{
		this->m_cellEditMode = ::get(arg<"celleditmode"_s>(), args..., default_(EditMode::ReadOnly));
	}

	virtual ~CBindTextColumn(void) = default;

	std::function<std::wstring(const T&)> GetGetter() const { return this->m_getFunction; }
	std::function<void(T&, const std::wstring&)> GetSetter() const { return this->m_setFunction; }

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
		return std::make_shared<CBindTextCell<T>>(
			this->m_pGrid, pRow, pColumn, 
			arg<"editmode"_s>() = this->m_cellEditMode);
	}
};

