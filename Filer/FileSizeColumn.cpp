#include "FileSizeColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "FileSizeCell.h"
#include "DriveSizeCell.h"
#include "PathCell.h"

std::shared_ptr<CCell> CFileSizeColumn::HeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CPathCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
}

std::shared_ptr<CCell> CFileSizeColumn::NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"Size");
}

std::shared_ptr<CCell> CFileSizeColumn::FilterCellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
}

std::shared_ptr<CCell> CFileSizeColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	if (auto pBindRow = dynamic_cast<CBindRow*>(pRow)) {
		auto spFile = pBindRow->GetItem<std::shared_ptr<CShellFile>>();
		if (auto spDrive = std::dynamic_pointer_cast<CDriveFolder>(spFile)) {
			return std::make_shared<CDriveSizeCell>(m_pSheet, pRow, pColumn, std::static_pointer_cast<FilerGridViewProperty>(m_pSheet->GetGridPtr()->GetGridViewPropPtr())->SizeCellPropPtr);
		}
	}

	return std::make_shared<CFileSizeCell>(m_pSheet, pRow, pColumn, std::static_pointer_cast<FilerGridViewProperty>(m_pSheet->GetGridPtr()->GetGridViewPropPtr())->SizeCellPropPtr);
}
