#pragma once
#include "MapColumn.h"
#include "HeaderSortCell.h"
#include "FilterCell.h"
#include "PathCell.h"
#include "FileLastWriteCell.h"
#include "GridView.h"
#include "CellProperty.h"
#include "FileSizeArgs.h"


struct FileTimeArgs;

template<typename T>
class CFileLastWriteColumn:public CMapColumn
{
private:
	std::shared_ptr<FileTimeArgs> m_spTimeArgs;
public:
	CFileLastWriteColumn(CGridView* pSheet = nullptr, std::shared_ptr<FileTimeArgs> spTimeProp = nullptr)
		:CMapColumn(pSheet), m_spTimeArgs(spTimeProp)
	{
	}

	~CFileLastWriteColumn(void) = default;

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CPathCell>(m_pGrid, pRow, pColumn);
	}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CHeaderSortCell>(m_pGrid, pRow, pColumn, arg<"text"_s>() = L"LastWrite");
	}

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFilterCell>(m_pGrid, pRow, pColumn);
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFileLastWriteCell<T>>(m_pGrid, pRow, pColumn);
	}
};

