#pragma once

#include "ParentMapColumn.h"

class CFileSizeColumn:public CParentDefaultMapColumn
{
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
		ar & boost::serialization::make_nvp("ParentDefaultMapColumn", boost::serialization::base_object<CParentDefaultMapColumn>(*this));
	}
public:
	CFileSizeColumn(CGridView* pGrid = nullptr);
	~CFileSizeColumn(void){}

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileSizeColumn* CloneRaw()const{return new CFileSizeColumn(*this);}
	std::shared_ptr<CFileSizeColumn> Clone()const{return std::shared_ptr<CFileSizeColumn>(CloneRaw());}

	cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

