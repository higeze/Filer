#pragma once
#include "Sheet.h"
#include "BindRow.h"
#include "MapColumn.h"
#include "CellProperty.h"
#include "FilerGridViewProperty.h"

class CFileSizeColumn:public CMapColumn
{
private:
	std::shared_ptr<FileSizeArgs> m_spSizeArgs;
public:
	CFileSizeColumn(CSheet* pSheet = nullptr, std::shared_ptr<FileSizeArgs> spSizeProp = nullptr)
		:CMapColumn(pSheet), m_spSizeArgs(spSizeProp)
	{
	}
	~CFileSizeColumn(void) = default;

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override;

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override;

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override;

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override;

	std::shared_ptr<FileSizeArgs> GetSizeArgsPtr() const { return m_spSizeArgs; }
};

