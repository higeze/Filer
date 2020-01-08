#pragma once

class CCell;
class CRow;
class CColumn;

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
	std::shared_ptr<CRow> m_pRow;
	CRowEventArgs(const std::shared_ptr<CRow>& pRow):m_pRow(pRow){}
	virtual ~CRowEventArgs(){}
};

/**
 *  CRowsEventArgs
 */
class CRowsEventArgs
{
public:
	std::vector<std::shared_ptr<CRow>> m_rows;
	CRowsEventArgs(const std::vector<std::shared_ptr<CRow>>& rows):m_rows(rows){}
	virtual ~CRowsEventArgs(){}
};

/**
 *  CColumnEventArgs
 */
class CColumnEventArgs
{
public:
	std::shared_ptr<CColumn> m_pColumn;
	CColumnEventArgs(const std::shared_ptr<CColumn>& spColumn):m_pColumn(spColumn){}
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
	CColumnMovedEventArgs(const std::shared_ptr<CColumn> spCol, const int from, const int to )
		:CColumnEventArgs(spCol), m_from(from), m_to(to){}
	virtual ~CColumnMovedEventArgs(){}
};

template<typename TRC>
class CMovedEventArgs
{
public:
	int m_from;
	int m_to;
	typename TRC::SharedPtr m_ptr;
public:
	CMovedEventArgs(typename TRC::SharedPtr ptr, int from, int to)
	:m_ptr(ptr), m_from(from), m_to(to){}
	virtual ~CMovedEventArgs() {}
};