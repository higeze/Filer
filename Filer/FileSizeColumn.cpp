#include "FileSizeColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileSizeCell.h"
#include "PathCell.h"

CFileSizeColumn::CFileSizeColumn(CGridView* pGrid, std::shared_ptr<FileSizeArgs> spSizeProp)
		:CParentDefaultMapColumn(pGrid), m_spSizeArgs(spSizeProp){}

CFileSizeColumn::cell_type CFileSizeColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

CFileSizeColumn::cell_type CFileSizeColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Size");
}

CFileSizeColumn::cell_type CFileSizeColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CFileSizeColumn::cell_type CFileSizeColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	//::OutputDebugStringA((boost::format("Col/Pointer:%1%\r\n") % pRow).str().c_str());
	return std::make_shared<CFileSizeCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}

