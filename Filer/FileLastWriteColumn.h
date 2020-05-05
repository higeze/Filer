#pragma once
#include "MapColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "PathCell.h"
#include "FileLastWriteCell.h"
#include "Sheet.h"
#include "CellProperty.h"
#include "FileSizeArgs.h"


struct FileTimeArgs;

template<typename... TItems>
class CFileLastWriteColumn:public CMapColumn
{
private:
	std::shared_ptr<FileTimeArgs> m_spTimeArgs;
public:
	CFileLastWriteColumn(CSheet* pSheet = nullptr, std::shared_ptr<FileTimeArgs> spTimeProp = nullptr)
		:CMapColumn(pSheet), m_spTimeArgs(spTimeProp)
	{
	}

	~CFileLastWriteColumn(void) = default;

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CPathCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"LastWrite");
	}

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFileLastWriteCell<TItems...>>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	std::shared_ptr<FileTimeArgs> GetTimeArgsPtr() { return m_spTimeArgs; }
};

