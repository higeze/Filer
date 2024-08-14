#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "MapColumn.h"
#include "HeaderSortCell.h"
#include "FilterCell.h"
#include "DriveSizeCell.h"
#include "PathCell.h"
#include "CellProperty.h"
#include "FilerGridViewProperty.h"
#include "FileSizeCell.h"

template<typename T> class CFileSizeCell;

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
		return std::make_shared<CPathCell>(m_pGrid, pRow, pColumn);
	}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CHeaderSortCell>(m_pGrid, pRow, pColumn, arg<"text"_s>() = L"Size");
	}

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFilterCell>(m_pGrid, pRow, pColumn);
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		if (auto p = dynamic_cast<CBindRow<T>*>(pRow)) {
			auto spFile = p->GetItem<std::shared_ptr<CShellFile>>();
			if (auto spDrive = std::dynamic_pointer_cast<CDriveFolder>(spFile)) {
				return std::make_shared<CDriveSizeCell<T>>(m_pGrid, pRow, pColumn);
			}
		}

		return static_pointer_cast<CCell>(std::make_shared<CFileSizeCell<T>>(m_pGrid, pRow, pColumn));
	}

	std::shared_ptr<FileSizeArgs> GetSizeArgsPtr() const { return m_spSizeArgs; }
};

