#pragma once

#include "FileNameColumn.h"

class CFileIconPathColumn : public CFileNameColumn
{
public:
	CFileIconPathColumn(CSheet* pSheet = nullptr);
	virtual ~CFileIconPathColumn(void) {};
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CFileNameColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileIconPathColumn* CloneRaw()const { return new CFileIconPathColumn(*this); }
	std::shared_ptr<CFileIconPathColumn> Clone()const { return std::shared_ptr<CFileIconPathColumn>(CloneRaw()); }

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

