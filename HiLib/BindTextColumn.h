#pragma once
#include "FilterCell.h"
#include "MapColumn.h"
#include "SortCell.h"
#include "named_arguments.h"

template<typename TItem>
class CBindTextColumn: public CMapColumn
{
private:
	std::wstring m_header;

	std::function<std::wstring(const TItem&)> m_getFunction;
	std::function<void(TItem&, const std::wstring&)> m_setFunction;

public:
	template<typename... Args>
	CBindTextColumn(CSheet* pSheet,
		const std::wstring& header,
		std::function<std::wstring(const TItem&)> getter,
		std::function<void(TItem&, const std::wstring&)> setter,
		Args... args)
		:CMapColumn(pSheet, args...), m_header(header), m_getFunction(getter), m_setFunction(setter)
	{}

	virtual ~CBindTextColumn(void) = default;

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CBindTextColumn* CloneRaw()const { return new CBindTextColumn(*this); }
	//std::shared_ptr<CToDoNameColumn> Clone()const { return std::shared_ptr<CToDoNameColumn>(CloneRaw()); }
	std::function<std::wstring(const TItem&)> GetGetter() { return m_getFunction; }
	std::function<void(TItem&, const std::wstring&)> GetSetter() { return m_setFunction; }

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
		return std::make_shared<CBindTextCell<TItem>>(
			m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty(),
			arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit);
	}
};

