#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "MapColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "FileSizeCell.h"
#include "DriveSizeCell.h"
#include "PathCell.h"
#include "CellProperty.h"
#include "FilerGridViewProperty.h"

template<typename T>
class CFileSizeColumn:public CMapColumn
{
private:
	std::shared_ptr<FileSizeArgs> m_spSizeArgs;
public:
	CFileSizeColumn(CGridView* pSheet = nullptr, std::shared_ptr<FileSizeArgs> spSizeProp = nullptr)
		:CMapColumn(pSheet), m_spSizeArgs(spSizeProp)
	{
	}
	~CFileSizeColumn(void) = default;

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CPathCell>(m_pGrid, pRow, pColumn, m_pGrid->GetCellProperty());
	}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CSortCell>(m_pGrid, pRow, pColumn, m_pGrid->GetHeaderProperty(), arg<"text"_s>() = L"Size");
	}

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFilterCell>(m_pGrid, pRow, pColumn, m_pGrid->GetFilterProperty());
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		if (auto p = dynamic_cast<CBindRow<T>*>(pRow)) {
			auto spFile = p->GetItem<std::shared_ptr<CShellFile>>();
			if (auto spDrive = std::dynamic_pointer_cast<CDriveFolder>(spFile)) {
				return std::make_shared<CDriveSizeCell<T>>(m_pGrid, pRow, pColumn, std::static_pointer_cast<FilerGridViewProperty>(m_pGrid->GetGridViewPropPtr())->SizeCellPropPtr);
			}
		}

		return std::make_shared<CFileSizeCell<T>>(m_pGrid, pRow, pColumn, std::static_pointer_cast<FilerGridViewProperty>(m_pGrid->GetGridViewPropPtr())->SizeCellPropPtr);
	}

	std::shared_ptr<FileSizeArgs> GetSizeArgsPtr() const { return m_spSizeArgs; }
};

