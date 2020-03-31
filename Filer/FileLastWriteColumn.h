#pragma once
#include "MapColumn.h"

struct FileTimeArgs;

class CFileLastWriteColumn:public CMapColumn
{
private:
	std::shared_ptr<FileTimeArgs> m_spTimeArgs;
public:
	CFileLastWriteColumn(CSheet* pSheet = nullptr, std::shared_ptr<FileTimeArgs> spTimeProp = nullptr);
	~CFileLastWriteColumn(void){}

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileLastWriteColumn* CloneRaw()const{return new CFileLastWriteColumn(*this);}
	virtual std::shared_ptr<CFileLastWriteColumn> Clone()const{return std::shared_ptr<CFileLastWriteColumn>(CloneRaw());}

	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;
	std::shared_ptr<FileTimeArgs> GetTimeArgsPtr() { return m_spTimeArgs; }
};

