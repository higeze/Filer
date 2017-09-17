#pragma once

#include "ParentMapColumn.h"

class CFileNameColumn: public CParentDefaultMapColumn
{
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
		ar & boost::serialization::make_nvp("ParentDefaultMapColumn", boost::serialization::base_object<CParentDefaultMapColumn>(*this));
	}
public:
	CFileNameColumn(CGridView* pGrid = nullptr);
	virtual ~CFileNameColumn(void){};
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CParentDefaultMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CFileNameColumn* CloneRaw()const{return new CFileNameColumn(*this);}
	std::shared_ptr<CFileNameColumn> Clone()const{return std::shared_ptr<CFileNameColumn>(CloneRaw());}

	cell_type HeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type FilterCellTemplate(CRow* pRow, CColumn* pColumn)override;
	cell_type CellTemplate(CRow* pRow, CColumn* pColumn)override;
};

