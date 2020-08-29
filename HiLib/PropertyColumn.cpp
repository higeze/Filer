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
#include "BindTextCell.h"

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
	return std::make_shared<CPropertyNameCell>(m_pSheet,pRow,pColumn,m_pSheet->GetFilterProperty());
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