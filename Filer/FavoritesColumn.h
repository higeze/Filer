#pragma once
#include "MapColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FavoriteCell.h"
#include "PathCell.h"
#include "CellProperty.h"

template<typename... TItems>
class CFavoritesColumn:public CMapColumn
{
public:
	CFavoritesColumn(CSheet* pSheet)
		:CMapColumn(pSheet, arg<"isminfit"_s>() = true)
	{}
	
	virtual ~CFavoritesColumn(void){}

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty());
	}

	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"Icon");
	}

	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CFavoriteCell<TItems...>>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	bool IsDragTrackable()const { return true; }
};

