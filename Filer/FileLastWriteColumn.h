#pragma once
#include "ParentMapColumn.h"

class CFileLastWriteColumn:public CParentDefaultMapColumn
{
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
		ar & boost::serialization::make_nvp("ParentDefaultMapColumn", boost::serialization::base_object<CParentDefaultMapColumn>(*this));
	}
public:
	CFileLastWriteColumn(CGridView* pGrid = nullptr);
	~CFileLastWriteColumn(void){}

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileLastWriteColumn* CloneRaw()const{return new CFileLastWriteColumn(*this);}
	virtual std::shared_ptr<CFileLastWriteColumn> Clone()const{return std::shared_ptr<CFileLastWriteColumn>(CloneRaw());}

	virtual cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	virtual cell_type CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

