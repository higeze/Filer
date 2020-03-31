#pragma once

#include "MapColumn.h"

struct FileSizeArgs;

class CFileSizeColumn:public CMapColumn
{
private:
	std::shared_ptr<FileSizeArgs> m_spSizeArgs;
public:
	CFileSizeColumn(CSheet* pSheet = nullptr, std::shared_ptr<FileSizeArgs> spSizeProp = nullptr);
	~CFileSizeColumn(void){}

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileSizeColumn* CloneRaw()const{return new CFileSizeColumn(*this);}
	std::shared_ptr<CFileSizeColumn> Clone()const{return std::shared_ptr<CFileSizeColumn>(CloneRaw());}

	std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;

	std::shared_ptr<FileSizeArgs> GetSizeArgsPtr() { return m_spSizeArgs; }
	//virtual SizingType GetSizingType()const { return SizingType::Fit; }
};

