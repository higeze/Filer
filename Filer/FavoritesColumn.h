#pragma once
#include "MapColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "GridView.h"
#include "FavoriteCell.h"
#include "PathCell.h"
#include "CellProperty.h"

template<typename T>
class CFavoritesColumn:public CMapColumn
{
public:
	CFavoritesColumn(CGridView* pSheet)
		:CMapColumn(pSheet, arg<"isminfit"_s>() = true)
	{}
	
	virtual ~CFavoritesColumn(void){}

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCell>(m_pGrid, pRow, pColumn, m_pGrid->GetHeaderProperty());
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CSortCell>(m_pGrid, pRow, pColumn, m_pGrid->GetHeaderProperty(), arg<"text"_s>() = L"Icon");
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFilterCell>(m_pGrid, pRow, pColumn, m_pGrid->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFavoriteCell<T>>(m_pGrid, pRow, pColumn, m_pGrid->GetCellProperty());
	}

	bool IsDragTrackable()const { return true; }
};

