#include "MapColumn.h"
#include "ChildIndexColumn.h"

class CPropertyNameColumn:public CMapColumn
{
public:
	CPropertyNameColumn::CPropertyNameColumn(CSheet* pSheet)
		:CMapColumn(pSheet, arg<"isminfit"_s>() = true, arg<"ismaxfit"_s>() = false){}
	virtual ~CPropertyNameColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

class CPropertyValueColumn:public CMapColumn
{
public:
	CPropertyValueColumn::CPropertyValueColumn(CSheet* pSheet)
		:CMapColumn(pSheet, arg<"isminfit"_s>() = true, arg<"ismaxfit"_s>() = false){}
	virtual ~CPropertyValueColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

