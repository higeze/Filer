#include "FileSizeColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "FilerGridView.h"
#include "FileSizeCell.h"
#include "PathCell.h"
#include "CellProperty.h"
#include "FilerGridViewProperty.h"

CFileSizeColumn::CFileSizeColumn(CGridView* pGrid, std::shared_ptr<FileSizeArgs> spSizeProp)
		:CParentDefaultMapColumn(pGrid), m_spSizeArgs(spSizeProp){}

std::shared_ptr<CCell> CFileSizeColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

std::shared_ptr<CCell> CFileSizeColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Size");
}

std::shared_ptr<CCell> CFileSizeColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileSizeColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileSizeCell>(m_pSheet,pRow,pColumn,std::static_pointer_cast<FilerGridViewProperty>(m_pSheet->GetGridPtr()->GetGridViewPropPtr())->SizeCellPropPtr);
}

