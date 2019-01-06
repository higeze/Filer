#include "ParentMapColumn.h"
#include "ChildIndexColumn.h"

class CParentPropertyNameColumn:public CParentMapColumn
{
public:
	CParentPropertyNameColumn(CGridView* pGrid);
	virtual ~CParentPropertyNameColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

class CChildPropertyNameColumn:public CChildIndexColumn
{
public:
	CChildPropertyNameColumn(CSheetCell* pSheetCell);
	virtual ~CChildPropertyNameColumn(){}
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};
#include "ChildRowHeaderColumn.h"
class CChildPropertyIndexColumn:public CChildRowHeaderColumn
{
public:
	CChildPropertyIndexColumn(CSheetCell* pSheetCell);
	virtual ~CChildPropertyIndexColumn(){}
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};


class CParentPropertyValueColumn:public CParentMapColumn
{
public:
	CParentPropertyValueColumn(CGridView* pGrid);
	virtual ~CParentPropertyValueColumn(){}
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};

class CChildPropertyValueColumn:public CChildIndexColumn
{
public:
	CChildPropertyValueColumn(CSheetCell* pSheetCell);
	virtual ~CChildPropertyValueColumn(){}
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);
};
