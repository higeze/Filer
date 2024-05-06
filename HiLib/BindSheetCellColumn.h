#pragma once
#include "BindGridView.h"
#include "BindTextCell.h"
#include "BindItemsSheetCell.h"
#include "FilterCell.h"
#include "MapColumn.h"
#include "HeaderSortCell.h"
#include "named_arguments.h"

template<typename TItem, typename TValueItem>
class CBindSheetCellColumn : public CMapColumn
{
private:
	std::wstring m_header;
	std::function<ReactiveVectorProperty<std::tuple<TValueItem>>&(std::tuple<TItem>&)> m_itemser;
	std::function<void(CBindItemsSheetCell<TItem, TValueItem>*)> m_initialize;
public:
	template<typename... Args>
	CBindSheetCellColumn(
		CGridView* pSheet,
		const std::wstring& header,
		std::function<ReactiveVectorProperty<std::tuple<TValueItem>>&(std::tuple<TItem>&)> itemser,
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
	std::function<ReactiveVectorProperty<std::tuple<TValueItem>>&(std::tuple<TItem>&)> GetItemser() const { return m_itemser; }

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCell>(m_pGrid, pRow, pColumn, m_pGrid->GetCellProperty());
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CHeaderSortCell>(m_pGrid, pRow, pColumn, m_pGrid->GetHeaderProperty(), arg<"text"_s>() = m_header);
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFilterCell>(m_pGrid, pRow, pColumn, m_pGrid->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		auto spCell = std::make_shared<CBindItemsSheetCell<TItem, TValueItem>>(
			m_pGrid, pRow, pColumn,
			m_pGrid->GetSheetProperty(),
			m_pGrid->GetCellProperty(),
			m_initialize);
		spCell->UpdateAll();
		return spCell;
	}
};

