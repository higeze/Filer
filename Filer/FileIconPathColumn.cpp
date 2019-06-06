#include "FileIconPathColumn.h"
#include "ParentColumnNameHeaderCell.h"
#include "FilterCell.h"
#include "Sheet.h"
#include "FileIconPathCell.h"
#include "PathCell.h"
#include "CellProperty.h"

CFileIconPathColumn::CFileIconPathColumn(CGridView* pGrid)
	:CFileNameColumn(pGrid)
{
}

std::shared_ptr<CCell> CFileIconPathColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFileIconPathCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}