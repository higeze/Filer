#pragma once
#include "MyWnd.h"
#include "MyPoint.h"
#include "Direct2DWrite.h"

#undef CreateEvent

namespace UIElementState
{
	enum Type{
		None,
		Normal,
		Hot,
		Pressed,
		PressedLeave,
		Dragged,
		Disabled
	};
}

struct EventArgs
{
public:
	CWnd* WndPtr;
	EventArgs(CWnd* pWnd = nullptr):WndPtr(pWnd){}
};

class CCell;

struct BeginEditEvent :public EventArgs
{
	CCell* CellPtr;
	BeginEditEvent(CWnd* pWnd, CCell* pCell)
		:EventArgs(pWnd), CellPtr(pCell) {}
};

struct EndEditEvent :public EventArgs
{
	EndEditEvent(CWnd* pWnd)
		:EventArgs() {}
};


struct CreateEvent :public EventArgs
{
	CreateEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:EventArgs(pWnd) {}
};


struct CloseEvent :public EventArgs
{

	CloseEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:EventArgs(pWnd) {}
};


struct KillFocusEvent :public EventArgs
{
	KillFocusEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:EventArgs(pWnd){}
};

struct KeyEventArgs:public EventArgs
{
	UINT Char;
	UINT RepeatCount;
	UINT Flags;
	KeyEventArgs(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:EventArgs(pWnd), Char(wParam), RepeatCount(lParam & 0xFF), Flags(lParam >> 16 & 0xFF) {}
};

struct CharEvent :public KeyEventArgs
{
	CharEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:KeyEventArgs(pWnd, wParam, lParam){}
};

struct KeyDownEvent :public KeyEventArgs
{
	KeyDownEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:KeyEventArgs(pWnd, wParam, lParam) {}
};

struct PaintEvent:public EventArgs
{
	PaintEvent(CWnd* pWnd)
		:EventArgs(pWnd){}
};

struct MouseEvent:public EventArgs
{
	UINT Flags;
	CPoint Point;
	MouseEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:EventArgs(pWnd), Flags(wParam),Point((short)LOWORD(lParam), (short)HIWORD(lParam)){}
	virtual ~MouseEvent(){}
};

struct LButtonDownEvent :public MouseEvent
{
	LButtonDownEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam):
		MouseEvent(pWnd, wParam, lParam){}
};

struct LButtonUpEvent :public MouseEvent
{
	LButtonUpEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :
		MouseEvent(pWnd, wParam, lParam) {}
};

struct LButtonClkEvent :public MouseEvent
{
	LButtonClkEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :MouseEvent(pWnd, wParam, lParam) {}
};

struct LButtonSnglClkEvent :public MouseEvent
{
	LButtonSnglClkEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :MouseEvent(pWnd, wParam, lParam) {}
};

struct LButtonDblClkEvent :public MouseEvent
{
	LButtonDblClkEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :MouseEvent(pWnd, wParam, lParam) {}
};

struct LButtonDblClkTimeExceedEvent :public MouseEvent
{
	LButtonDblClkTimeExceedEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :MouseEvent(pWnd, wParam, lParam) {}
};

struct LButtonBeginDragEvent :public MouseEvent
{
	LButtonBeginDragEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :MouseEvent(pWnd, wParam, lParam) {}
};

struct RButtonDownEvent :public MouseEvent
{
	RButtonDownEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :MouseEvent(pWnd, wParam, lParam) {}
};

struct MouseMoveEvent :public MouseEvent
{
	MouseMoveEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :
		MouseEvent(pWnd, wParam, lParam) {}
};

struct MouseLeaveEvent :public MouseEvent
{
	MouseLeaveEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam) :MouseEvent(pWnd, wParam, lParam) {}
};


struct MouseWheelEvent:public MouseEvent
{
	short Delta;
	MouseWheelEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:MouseEvent(pWnd, wParam, lParam), Delta(GET_WHEEL_DELTA_WPARAM(wParam)){}
};

struct SetCursorEvent:public EventArgs
{
	UINT HitTest;
	BOOL& Handled;
	SetCursorEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		:EventArgs(pWnd), HitTest(LOWORD(lParam)), Handled(bHandled){}
};

struct SetFocusEvent :public EventArgs
{
	SetFocusEvent(CWnd* pWnd):EventArgs(pWnd){}
};

struct ContextMenuEvent:public EventArgs
{
public:
	CPoint Point;
	ContextMenuEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:EventArgs(pWnd),Point((short)LOWORD(lParam), (short)HIWORD(lParam)){}
};


class CUIElement
{
protected:
	UIElementState::Type m_state;

public:
	CUIElement()
		:m_state(UIElementState::Normal){}
	virtual ~CUIElement(){}

	virtual UIElementState::Type GetState()const{return m_state;}
	void SetState(const UIElementState::Type& state);

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e) {}
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) {}
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) {}

	virtual void OnMButtonDown(const MouseEvent& e){}//TODO
	virtual void OnMButtonUp(const MouseEvent& e){}//TODO

	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseEnter(const MouseEvent& e);//TODO
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e){}

	virtual void OnKeyDown(const KeyDownEvent& e){}
	virtual void OnContextMenu(const ContextMenuEvent& e){}

	virtual void OnPropertyChanged(const wchar_t* name){}
};
