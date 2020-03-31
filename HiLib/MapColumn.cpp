#include "MapColumn.h"
#include "Sheet.h"
#include "GridView.h"
#include "ParentColumnNameHeaderCell.h"
#include "ParentColumnHeaderCell.h"
#include "ParentColumnHeaderHeaderCell.h"

#include "FilterCell.h"
#include "Cell.h"
#include "Row.h"
#include "CellProperty.h"

std::shared_ptr<CCell>& CMapColumn::Cell(CRow* pRow )
{
	//TODO Sheet should check if Row or Column hasCell and get Cell.

	if (pRow->HasCell()) {
		return pRow->Cell(this);
	} else {

		auto iter = m_mapCell.find(pRow);
		if (iter != m_mapCell.end()) {
			return iter->second;
		} else {
			if (pRow == m_pSheet->GetHeaderRowPtr().get()) {
				return m_mapCell.insert(std::make_pair(pRow, HeaderCellTemplate(pRow, this))).first->second;
			} else if (pRow == m_pSheet->GetNameHeaderRowPtr().get()) {
				return m_mapCell.insert(std::make_pair(pRow, NameHeaderCellTemplate(pRow, this))).first->second;
			} else if (pRow == m_pSheet->GetFilterRowPtr().get()) {
				return m_mapCell.insert(std::make_pair(pRow, FilterCellTemplate(pRow, this))).first->second;
			} else {
				return m_mapCell.insert(std::make_pair(pRow, CellTemplate(pRow, this))).first->second;
			}
		}
	}
}

std::shared_ptr<CCell> CMapColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderIndexCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CMapColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnNameHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CMapColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CMapColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CTextCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}