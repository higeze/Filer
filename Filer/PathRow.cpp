#include "PathRow.h"
#include "Column.h"
#include "GridView.h"
#include "PathCell.h"
#include "CellProperty.h"


std::shared_ptr<CCell>& CPathRow::Cell(CColumn* pCol)
{
	if (!pCol) {
		throw std::exception("Error in CPathRow::Cell");
	}else if (pCol == m_pGrid->GetHeaderColumnPtr().get()) {
		if (!m_spHeaderCell) {
			m_spHeaderCell = std::make_shared<CCell>(m_pGrid, this, pCol, m_pGrid->GetHeaderProperty());
		}
		return m_spHeaderCell;
	} else {
		if (!m_spPathCell) {
			m_spPathCell = std::make_shared<CPathCell>(m_pGrid, this, pCol, m_pGrid->GetCellProperty());
		}
		return m_spPathCell;
	}
}
