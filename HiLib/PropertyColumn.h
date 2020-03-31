#include "MapColumn.h"
#include "ChildIndexColumn.h"

class CParentPropertyNameColumn:public CMapColumn
{
public:
	CParentPropertyNameColumn(CSheet* pSheet);
	virtual ~CParentPropertyNameColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

class CChildPropertyNameColumn:public CChildIndexColumn
{
public:
	CChildPropertyNameColumn(CSheet* pSheet);
	virtual ~CChildPropertyNameColumn(){}
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};
#include "ChildRowHeaderColumn.h"
class CChildPropertyIndexColumn:public CChildRowHeaderColumn
{
public:
	CChildPropertyIndexColumn(CSheet* pSheet);
	virtual ~CChildPropertyIndexColumn(){}
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};


class CParentPropertyValueColumn:public CMapColumn
{
public:
	CParentPropertyValueColumn(CSheet* pSheet);
	virtual ~CParentPropertyValueColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

class CChildPropertyValueColumn:public CChildIndexColumn
{
public:
	CChildPropertyValueColumn(CSheet* pSheet);
	virtual ~CChildPropertyValueColumn(){}
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};
