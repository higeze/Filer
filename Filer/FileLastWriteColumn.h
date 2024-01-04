#pragma once
#include "MapColumn.h"
#include "CellProperty.h"
#include "FileSizeArgs.h"


struct FileTimeArgs;

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

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override;
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override;
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override;
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override;

	std::shared_ptr<FileTimeArgs> GetTimeArgsPtr() const { return m_spTimeArgs; }
};

