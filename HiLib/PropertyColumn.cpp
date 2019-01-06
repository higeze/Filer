#include "PropertyColumn.h"
#include "Sheet.h"
#include "ParentColumnHeaderCell.h"
#include "ParentColumnNameHeaderCell.h"
#include "PropertyNameCell.h"
#include "FilterCell.h"
#include "Cell.h"
#include "TextCell.h"
#include "MyString.h"
#include "Row.h"
#include "GridView.h"


CParentPropertyNameColumn::CParentPropertyNameColumn(CGridView* pGrid)
	:CParentMapColumn(pGrid)
{}

CParentPropertyNameColumn::std::shared_ptr<CCell> CParentPropertyNameColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Property");
}

CParentPropertyNameColumn::std::shared_ptr<CCell> CParentPropertyNameColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CParentPropertyNameColumn::std::shared_ptr<CCell> CParentPropertyNameColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPropertyNameCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}

CChildPropertyNameColumn::CChildPropertyNameColumn(CSheetCell* pSheetCell)
	:CChildIndexColumn(pSheetCell)
{}

CChildPropertyNameColumn::std::shared_ptr<CCell> CChildPropertyNameColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Property");
}

CChildPropertyNameColumn::std::shared_ptr<CCell> CChildPropertyNameColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CChildPropertyNameColumn::std::shared_ptr<CCell> CChildPropertyNameColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPropertyNameCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}

CChildPropertyIndexColumn::CChildPropertyIndexColumn(CSheetCell* pSheetCell)
	:CChildRowHeaderColumn(pSheetCell)
{}

CChildPropertyIndexColumn::std::shared_ptr<CCell> CChildPropertyIndexColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"X");
}

CChildPropertyIndexColumn::std::shared_ptr<CCell> CChildPropertyIndexColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CChildPropertyIndexColumn::std::shared_ptr<CCell> CChildPropertyIndexColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),boost::lexical_cast<std::wstring>(pRow->GetIndex<AllTag>()));
}

CParentPropertyValueColumn::CParentPropertyValueColumn(CGridView* pGrid)
	:CParentMapColumn(pGrid)
{}

CParentPropertyValueColumn::std::shared_ptr<CCell> CParentPropertyValueColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CParentColumnHeaderStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Value");
}

CParentPropertyValueColumn::std::shared_ptr<CCell> CParentPropertyValueColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CParentPropertyValueColumn::std::shared_ptr<CCell> CParentPropertyValueColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CEditableNoWrapStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty(),L"");
}

CChildPropertyValueColumn::CChildPropertyValueColumn(CSheetCell* pSheetCell)
	:CChildIndexColumn(pSheetCell)
{
	m_lineType = LineType::OneLine;
}

CChildPropertyValueColumn::std::shared_ptr<CCell> CChildPropertyValueColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(),L"Value");
}

CChildPropertyValueColumn::std::shared_ptr<CCell> CChildPropertyValueColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

CChildPropertyValueColumn::std::shared_ptr<CCell> CChildPropertyValueColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CEditableNoWrapStringCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty(),L"44");
}