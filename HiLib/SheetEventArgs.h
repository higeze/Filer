#pragma once


/**
 *  CellEventArgs
 */
struct CellEventArgs
{
public:
	CCell* CellPtr;
	CellEventArgs(CCell* pCell):CellPtr(pCell){}
	virtual ~CellEventArgs(){}
};
/**
 *  CellEventArgs
 */
struct CellContextMenuEventArgs:public CellEventArgs
{
public:
	CPoint Point;
	CellContextMenuEventArgs(CCell* pCell,CPoint pt)
		:CellEventArgs(pCell),Point(pt){}
	virtual ~CellContextMenuEventArgs(){}

};
/**
 *  CRowEventArgs
 */
class CRowEventArgs
{
public:
	CRow* m_pRow;
	CRowEventArgs(CRow* pRow):m_pRow(pRow){}
	virtual ~CRowEventArgs(){}
};

/**
 *  CRowsEventArgs
 */
class CRowsEventArgs
{
public:
	std::vector<CRow*> m_rows;
	CRowsEventArgs(const std::vector<CRow*> rows):m_rows(rows){}
	virtual ~CRowsEventArgs(){}
};

/**
 *  CColumnEventArgs
 */
class CColumnEventArgs
{
public:
	CColumn* m_pColumn;
	CColumnEventArgs(CColumn* pColumn):m_pColumn(pColumn){}
	virtual ~CColumnEventArgs(){}
};

/**
 *  CColumnMovedEventArgs
 */
class CColumnMovedEventArgs:public CColumnEventArgs
{
public:
	int m_from;
	int m_to;
public:
	CColumnMovedEventArgs(CColumn* pColumn, int from, int to )
		:CColumnEventArgs(pColumn), m_from(from), m_to(to){}
	virtual ~CColumnMovedEventArgs(){}
};

template<typename TRC>
class CMovedEventArgs
{
public:
	int m_from;
	int m_to;
	TRC::template Ptr m_ptr;
public:
	CMovedEventArgs(TRC::template Ptr ptr, int from, int to)
	:m_ptr(ptr), m_from(from), m_to(to){}
	virtual ~CMovedEventArgs() {}
};