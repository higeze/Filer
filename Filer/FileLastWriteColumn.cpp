#include "FileLastWriteColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "PathCell.h"
#include "FileLastWriteCell.h"
#include "Sheet.h"
#include "CellProperty.h"
#include "FileSizeArgs.h"


CFileLastWriteColumn::CFileLastWriteColumn(CSheet* pSheet, std::shared_ptr<FileTimeArgs> spTimeProp)
	:CMapColumn(pSheet), m_spTimeArgs(spTimeProp){}

std::shared_ptr<CCell> CFileLastWriteColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());	
}

std::shared_ptr<CCell> CFileLastWriteColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"LastWrite");
}

std::shared_ptr<CCell> CFileLastWriteColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileLastWriteColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileLastWriteCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}
