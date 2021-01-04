#pragma once
#include "Sheet.h"
#include "MapColumn.h"
#include "SortCell.h"
#include "FilterCell.h"
#include "FileSizeCell.h"
#include "PathCell.h"
#include "CellProperty.h"
#include "FilerGridViewProperty.h"

template<typename... TItems>
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

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CPathCell>(m_pSheet, pRow, pColumn, m_pSheet->GetCellProperty());
	}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CSortCell>(m_pSheet, pRow, pColumn, m_pSheet->GetHeaderProperty(), arg<"text"_s>() = L"Size");
	}

	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFilterCell>(m_pSheet, pRow, pColumn, m_pSheet->GetFilterProperty());
	}

	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) override
	{
		return std::make_shared<CFileSizeCell<TItems...>>(m_pSheet, pRow, pColumn, std::static_pointer_cast<FilerGridViewProperty>(m_pSheet->GetGridPtr()->GetGridViewPropPtr())->SizeCellPropPtr);
	}

	std::shared_ptr<FileSizeArgs> GetSizeArgsPtr() const { return m_spSizeArgs; }
};

