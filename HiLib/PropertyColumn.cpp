#include "PropertyColumn.h"
#include "Sheet.h"
#include "SortCell.h"
#include "PropertyNameCell.h"
#include "FilterCell.h"
#include "Cell.h"
#include "TextCell.h"
#include "MyString.h"
#include "Row.h"
#include "GridView.h"
#include "CellProperty.h"
#include "RowIndexCell.h"


CPropertyNameColumn::CPropertyNameColumn(CSheet* pSheet)
	:CMapColumn(pSheet)
{}

std::shared_ptr<CCell> CPropertyNameColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"Name");
}

std::shared_ptr<CCell> CPropertyNameColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CPropertyNameColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPropertyNameCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}


CPropertyValueColumn::CPropertyValueColumn(CSheet* pSheet)
	:CMapColumn(pSheet)
{
	m_isMinLengthFit = true;
}

std::shared_ptr<CCell> CPropertyValueColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet,pRow,pColumn,m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"Value");
}

std::shared_ptr<CCell> CPropertyValueColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CPropertyValueColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CTextCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty(),
		arg<"editmode"_s>() = EditMode::LButtonDownEdit,
		arg<"text"_s>() = L"");
}

CPropertyIndexColumn::CPropertyIndexColumn(CSheet* pSheet)
	:CMapColumn(pSheet)
{
	m_isMinLengthFit = true;
}

std::shared_ptr<CCell> CPropertyIndexColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"Index");
}

std::shared_ptr<CCell> CPropertyIndexColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty());
}

std::shared_ptr<CCell> CPropertyIndexColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CRowIndexCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty());
}