#include "ParentMapColumn.h"
#include "Sheet.h"
#include "GridView.h"
#include "ParentColumnNameHeaderCell.h"
#include "ParentColumnHeaderCell.h"
#include "ParentColumnHeaderHeaderCell.h"

#include "FilterCell.h"
#include "Cell.h"
#include "Row.h"
#include "CellProperty.h"

std::shared_ptr<CCell>& CParentMapColumn::Cell(CRow* pRow )
{
	//TODO Sheet should check if Row or Column hasCell and get Cell.

	if (pRow->HasCell()) {
		return pRow->Cell(this);
	} else {

		auto iter = m_mapCell.find(pRow);
		if (iter != m_mapCell.end()) {
			return iter->second;
		} else {
			auto pGrid = static_cast<CGridView*>(m_pSheet);
			if (pRow == pGrid->GetHeaderRowPtr().get()) {
				return m_mapCell.insert(std::make_pair(pRow, HeaderCellTemplate(pRow, this))).first->second;
			} else if (pRow == pGrid->GetNameHeaderRowPtr().get()) {
				return m_mapCell.insert(std::make_pair(pRow, NameHeaderCellTemplate(pRow, this))).first->second;
			} else if (pRow == pGrid->GetHeaderHeaderRowPtr().get()) {
				return m_mapCell.insert(std::make_pair(pRow, HeaderHeaderCellTemplate(pRow, this))).first->second;
			} else if (pRow == pGrid->GetFilterRowPtr().get()) {
				return m_mapCell.insert(std::make_pair(pRow, FilterCellTemplate(pRow, this))).first->second;
			} else {
				return m_mapCell.insert(std::make_pair(pRow, CellTemplate(pRow, this))).first->second;
			}
		}
	}
}

std::shared_ptr<CCell> CParentMapColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderIndexCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CParentMapColumn::HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CParentMapColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnNameHeaderCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CParentMapColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CParentMapColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CTextCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}

std::shared_ptr<CCell> CParentDefaultMapColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentDefaultColumnHeaderIndexCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty());
}