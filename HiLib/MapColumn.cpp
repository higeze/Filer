#include "MapColumn.h"
#include "GridView.h"
#include "ColumnIndexCell.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "Cell.h"
#include "Row.h"
#include "CellProperty.h"

//std::shared_ptr<CCell>& CMapColumn::Cell(CRow* pRow )
//{
//	//TODO Sheet should check if Row or Column hasCell and get Cell.
//
//	//if (pRow->HasCell()) {
//	//	return pRow->Cell(this);
//	//} else {
//
//	//	auto iter = m_mapCell.find(pRow);
//	//	if (iter != m_mapCell.end()) {
//	//		return iter->second;
//	//	} else {
//	//		if (pRow == m_pGrid->GetHeaderRowPtr().get()) {
//	//			return m_mapCell.insert(std::make_pair(pRow, HeaderCellTemplate(pRow, this))).first->second;
//	//		} else if (pRow == m_pGrid->GetNameHeaderRowPtr().get()) {
//	//			return m_mapCell.insert(std::make_pair(pRow, NameHeaderCellTemplate(pRow, this))).first->second;
//	//		} else if (pRow == m_pGrid->GetFilterRowPtr().get()) {
//	//			return m_mapCell.insert(std::make_pair(pRow, FilterCellTemplate(pRow, this))).first->second;
//	//		} else {
//	//			return m_mapCell.insert(std::make_pair(pRow, CellTemplate(pRow, this))).first->second;
//	//		}
//	//	}
//	//}
//	if (pRow->HasCell()) {
//		return pRow->Cell(this);
//	} else {
//		auto& container = m_pGrid->m_allCells.get<rowcol_tag>();
//		auto iter = container.find(MAKELONGLONG(pRow, this));
//		if (iter == container.end()) {
//			if (pRow == m_pGrid->GetHeaderRowPtr().get()) {
//				iter = container.insert(HeaderCellTemplate(pRow, this)).first;
//			} else if (pRow == m_pGrid->GetNameHeaderRowPtr().get()) {
//				iter = container.insert(NameHeaderCellTemplate(pRow, this)).first;
//			} else if (pRow == m_pGrid->GetFilterRowPtr().get()) {
//				iter = container.insert(FilterCellTemplate(pRow, this)).first;
//			} else {
//				iter = container.insert(CellTemplate(pRow, this)).first;
//			}
//		}
//		return const_cast<std::shared_ptr<CCell>&>(*iter);
//	}
//
//}

//void CMapColumn::Clear()
//{
//	auto& container = m_pGrid->m_allCells.get<col_tag>();
//	auto pair = container.equal_range((CColumn*)(this));
//	container.erase(pair.first, pair.second);
//}

//void CMapColumn::Erase(const CRow* pRow) 
//{
//	auto& container = m_pGrid->m_allCells.get<row_tag>();
//	auto pair = container.equal_range(const_cast<CRow*>(pRow));
//	container.erase(pair.first, pair.second);
//}

std::shared_ptr<CCell> CMapColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CColumnIndexCell>(m_pGrid,pRow,pColumn,m_pGrid->GetHeaderProperty());
}

std::shared_ptr<CCell> CMapColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pGrid,pRow,pColumn,m_pGrid->GetHeaderProperty());
}

std::shared_ptr<CCell> CMapColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pGrid,pRow,pColumn,m_pGrid->GetFilterProperty());
}

std::shared_ptr<CCell> CMapColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CTextCell>(m_pGrid,pRow,pColumn,m_pGrid->GetCellProperty());
}