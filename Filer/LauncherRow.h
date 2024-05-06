#pragma once
#include "GridView.h"
#include "MapRow.h"
#include "LauncherCell.h"
#include "CellProperty.h"

template<typename T>
class CLauncherRow:public CMapRow
{
protected:
	std::unordered_map<const CColumn*,std::shared_ptr<CCell>> m_mapCell; /**<Map to hold cell*/

public:
	CLauncherRow(CGridView* pSheet)
		:CMapRow(pSheet){}
	
	virtual ~CLauncherRow(void) = default;

	virtual bool HasCell()const { return true; }

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CLauncherCell<T>>(m_pGrid, pRow, pColumn);
	}

	std::shared_ptr<CCell>& Cell(CColumn* pColumn ) override
	{
		auto iter = m_mapCell.find(pColumn);
		if (iter != m_mapCell.end()) {
			return iter->second;
		} else {
			return m_mapCell.insert(std::make_pair(pColumn, CellTemplate(this, pColumn))).first->second;
		}
	}

	bool IsDragTrackable()const { return true; }
};

