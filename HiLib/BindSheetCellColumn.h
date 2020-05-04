#pragma once
#include "BindGridView.h"
#include "BindTextCell.h"
#include "BindItemsSheetCell.h"
#include "FilterCell.h"
#include "MapColumn.h"
#include "SortCell.h"
#include "named_arguments.h"

template<typename TItem, typename TValueItem>
class CBindSheetCellColumn : public CMapColumn
{
private:
	std::wstring m_header;
	std::function<observable_vector<std::tuple<TValueItem>>&(TItem&)> m_itemser;
	std::function<void(CBindItemsSheetCell<TItem, TValueItem>*)> m_initialize;
public:
	template<typename... Args>
	CBindSheetCellColumn(CSheet* pSheet,
		const std::wstring& header,
		std::function<observable_vector<std::tuple<TValueItem>>&(TItem&)> itemser,
		std::function<void(CBindItemsSheetCell<TItem, TValueItem>*)> initializer,
		Args... args)
		:CMapColumn(pSheet, args...), m_header(header), m_itemser(itemser), m_initialize(initializer)
	{
		m_isMinLengthFit = true;
		m_isMaxLengthFit = false;
	}

//	virtual SizingType GetSizingType()const override { return SizingType::AlwaysFit; }

	virtual ~CBindSheetCellColumn(void) = default;

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CBindSheetCellColumn* CloneRaw()const { return new CBindSheetCellColumn(*this); }
	//std::shared_ptr<CToDoNameColumn> Clone()const { return std::shared_ptr<CToDoNameColumn>(CloneRaw()); }
	std::function<observable_vector<TValueItem>&(TItem&)> GetItemser() { return m_itemser; }

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
		auto spCell = std::make_shared<CBindItemsSheetCell<TItem, TValueItem>>(
			m_pSheet, pRow, pColumn,
			m_pSheet->GetSheetProperty(),
			m_pSheet->GetCellProperty(),
			m_initialize);
		spCell->UpdateAll();
		return spCell;
	}
};

