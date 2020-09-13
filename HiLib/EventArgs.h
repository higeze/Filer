#pragma once

#include "MyRect.h"
#include "MyPoint.h"
#include "MyDC.h"

class IWnd;
class CCell;
class CColumn;
class CRow;

class Event
{
public:
	typedef IWnd window_type;
	typedef int int;
public:
	window_type* m_pParentControl;
	Event(window_type* pWnd):m_pParentControl(pWnd){}
	virtual ~Event(){}
};
class CRowEventArgs
{
public:
	CRow* m_pRow;
	CRowEventArgs(CRow* pRow):m_pRow(pRow){}
	virtual ~CRowEventArgs(){]
};
class CColumnEventArgs
{
public:
	CColumn* m_pColumn;
	CColumnEventArgs(CColumn* pColumn):m_pColumn(pColumn){}
	virtual ~CColumnEventArgs(){}
};

class CellEventArgs:public Event
{
public:
	typedef CCell std::shared_ptr<CCell>;
public:
	std::shared_ptr<CCell>* m_pCell;
	CellEventArgs(window_type* pWnd,std::shared_ptr<CCell>* pCell):Event(pWnd),m_pCell(pCell){}
	virtual ~CellEventArgs(){}
};

class CellContextMenuEventArgs:public CellEventArgs
{
public:
	CPoint m_ptClient;
	CellContextMenuEventArgs(window_type* pWnd,std::shared_ptr<CCell>* pCell,CPoint ptClient)
		:CellEventArgs(pWnd,pCell),m_ptClient(ptClient){}
	virtual CellContextMenuEventArgs(){}
};

//class PaintEventArgs:public EventArgs
//{
//public:
//	CDC* m_pDC;
//public:
//	PaintEventArgs(window_type* pWnd,CDC* pDC)
//		:EventArgs(pWnd),m_pDC(pDC){}
//	virtual ~PaintEventArgs(){}
//};

//class MouseEventArgs:public EventArgs
//{
//public:
//	UINT m_uFlags;
//	CPoint m_ptClient;
//public:
//	MouseEventArgs(window_type* pWnd,UINT uFlags,CPoint ptClient)
//		:EventArgs(pWnd),m_uFlags(uFlags),m_ptClient(ptClient){}
//	virtual ~MouseEventArgs(){}
//};
//
//class KeyEventArgs:public EventArgs
//{
//public:
//	UINT m_uChar;
//	UINT m_uRepCnt;
//	UINT m_uFlags;
//public:
//	KeyEventArgs(window_type* pWnd,UINT uChar,UINT uRepCnt,UINT uFlags)
//		:EventArgs(pWnd),m_uChar(uChar),m_uRepCnt(uRepCnt),m_uFlags(uFlags){}
//	virtual ~KeyEventArgs(){}
//};

//class CContextMenuEventArgs:public EventArgs
//{
//public:
//	CPoint m_ptClient;
//	CContextMenuEventArgs(window_type* pWnd,CPoint ptClient)
//		:EventArgs(pWnd),m_ptClient(ptClient){}
//	virtual ~CContextMenuEventArgs(){}
//};