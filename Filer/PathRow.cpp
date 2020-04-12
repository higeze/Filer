#include "PathRow.h"
#include "Column.h"
#include "Sheet.h"
#include "PathCell.h"
#include "CellProperty.h"


std::shared_ptr<CCell>& CPathRow::Cell(CColumn* pCol)
{
	if (!pCol) {
		throw std::exception("Error in CPathRow::Cell");
	}else if (pCol == m_pSheet->GetHeaderColumnPtr().get()) {
		if (!m_spHeaderCell) {
			m_spHeaderCell = std::make_shared<CCell>(m_pSheet, this, pCol, m_pSheet->GetHeaderProperty());
		}
		return m_spHeaderCell;
	} else {
		if (!m_spPathCell) {
			m_spPathCell = std::make_shared<CPathCell>(m_pSheet, this, pCol, m_pSheet->GetCellProperty());
		}
		return m_spPathCell;
	}
}
