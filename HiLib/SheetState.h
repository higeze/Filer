#pragma once
#include "RowColumn.h"
#include "MyPoint.h"
class CSheet;
class ICursor;
class ITracker;
class IDragger;

struct MouseEventArgs;
struct SetCursorEventArgs;

class CSheetStateMachine
{
private:
	std::shared_ptr<ICursor> m_cursor;
	std::shared_ptr<ITracker> m_rowTracker;
	std::shared_ptr<ITracker> m_colTracker;
	std::shared_ptr<IDragger> m_rowDragger;
	std::shared_ptr<IDragger> m_colDragger;
	std::shared_ptr<IDragger> m_itemDragger;

public:
	CSheetStateMachine(
		std::shared_ptr<ICursor> cursor,
		std::shared_ptr<ITracker> rowTracker,
		std::shared_ptr<ITracker> colTracker,
		std::shared_ptr<IDragger> rowDragger,
		std::shared_ptr<IDragger> colDragger,
		std::shared_ptr<IDragger> itemDragger) 
	:m_cursor(cursor),
	m_rowTracker(rowTracker),
	m_colTracker(colTracker),
	m_rowDragger(rowDragger),
	m_colDragger(colDragger),
	m_itemDragger(itemDragger){}

	std::shared_ptr<ICursor> Cursor() { return m_cursor; }
	std::shared_ptr<ITracker> RowTracker() { return m_rowTracker; }
	std::shared_ptr<ITracker> ColTracker() { return m_colTracker; }
	std::shared_ptr<IDragger> RowDragger() { return m_rowDragger; }
	std::shared_ptr<IDragger> ColDragger() { return m_colDragger; }
	std::shared_ptr<IDragger> ItemDragger() { return m_itemDragger; }
};

class ISheetState
{
public:
	ISheetState(){}
	virtual ~ISheetState(){}

	virtual void Entry(CSheet* pSheet, MouseEventArgs& e) {}
	virtual void Exit(CSheet* pSheet, MouseEventArgs& e) {}
	virtual ISheetState* ChangeState(CSheet* pSheet, IMouseState* pMouseState, MouseEventArgs& e);
	ISheetState* ISheetState::ChangeState(CSheet* pSheet, IMouseState* pMouseState, MouseEventArgs& e)
	{
		this->Exit(pSheet, e);
		pMouseState->Entry(pSheet, e);
		return pMouseState;
	}

	ISheetState* IMouseState::KeepState()
	{
		return this;
	}

	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)=0;
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)=0;
	virtual ISheetState* OnLButtonSnglClk(CSheet* pSheet, MouseEventArgs& e) = 0;
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)=0;
	virtual ISheetState* OnLButtonBeginDrag(CSheet* pSheet, MouseEventArgs& e) = 0;
	virtual ISheetState* OnLButtonEndDrag(CSheet* pSheet, MouseEventArgs& e) = 0;

	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)=0;

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e)=0;
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)=0;

	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)=0;
};

class CNormalState:public ISheetState
{
private:
	CRowColumn m_rocoMouse;
public:
	CNormalState():m_rocoMouse(){}
	virtual ~CNormalState(){}

	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonSnglClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonBeginDrag(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonEndDrag(CSheet* pSheet, MouseEventArgs& e);


	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);
};

class CTrackingState:public ISheetState
{
public:
	CTrackingState(){}
	virtual ~CTrackingState(){}

	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonSnglClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonBeginDrag(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonEndDrag(CSheet* pSheet, MouseEventArgs& e);


	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);

};

class CColumnDraggingState:public ISheetState
{
public:
	CColumnDraggingState(){}
	virtual ~CColumnDraggingState(){}

	static ISheetState* State();

	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonSnglClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonBeginDrag(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonEndDrag(CSheet* pSheet, MouseEventArgs& e);


	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);

};

class CRowDraggingState :public ISheetState
{
public:
	CRowDraggingState() {}
	virtual ~CRowDraggingState() {}

	static ISheetState* State();

	virtual ISheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonSnglClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonBeginDrag(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnLButtonEndDrag(CSheet* pSheet, MouseEventArgs& e);


	virtual ISheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);

	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
	virtual ISheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);

};


