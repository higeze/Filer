#include "FileSizeColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "FilerGridView.h"
#include "FileSizeCell.h"
#include "PathCell.h"
#include "CellProperty.h"
#include "FilerGridViewProperty.h"

CFileSizeColumn::CFileSizeColumn(CSheet* pSheet, std::shared_ptr<FileSizeArgs> spSizeProp)
		:CMapColumn(pSheet), m_spSizeArgs(spSizeProp){}

std::shared_ptr<CCell> CFileSizeColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

std::shared_ptr<CCell> CFileSizeColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),arg<"text"_s>() = L"Size");
}

std::shared_ptr<CCell> CFileSizeColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileSizeColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileSizeCell>(m_pSheet,pRow,pColumn,std::static_pointer_cast<FilerGridViewProperty>(m_pSheet->GetGridPtr()->GetGridViewPropPtr())->SizeCellPropPtr);
}

