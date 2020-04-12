#include "MapColumn.h"
#include "ChildIndexColumn.h"

class CPropertyNameColumn:public CMapColumn
{
public:
	CPropertyNameColumn(CSheet* pSheet);
	virtual ~CPropertyNameColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

class CPropertyIndexColumn :public CMapColumn
{
public:
	CPropertyIndexColumn(CSheet* pSheet);
	virtual ~CPropertyIndexColumn() {}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};


class CPropertyValueColumn:public CMapColumn
{
public:
	CPropertyValueColumn(CSheet* pSheet);
	virtual ~CPropertyValueColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

