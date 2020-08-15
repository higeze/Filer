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
	BOOL* HandledPtr;
	EventArgs(CWnd* pWnd = nullptr, BOOL* pHandled = nullptr):WndPtr(pWnd), HandledPtr(pHandled){}
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
	CreateEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled) {}
};

struct CloseEvent :public EventArgs
{
	CloseEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam)
		:EventArgs(pWnd) {}
};

struct KillFocusEvent :public EventArgs
{
	KillFocusEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled){}
};

struct KeyEventArgs:public EventArgs
{
	UINT Char;
	UINT RepeatCount;
	UINT Flags;
	KeyEventArgs(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled), Char(wParam), RepeatCount(lParam & 0xFF), Flags(lParam >> 16 & 0xFF) {}
};

struct CharEvent :public KeyEventArgs
{
	CharEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:KeyEventArgs(pWnd, wParam, lParam, pHandled){}
};

struct KeyDownEvent :public KeyEventArgs
{
	KeyDownEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:KeyEventArgs(pWnd, wParam, lParam, pHandled) {}
};

struct SysKeyDownEvent :public KeyDownEvent
{
	using KeyDownEvent::KeyDownEvent;
};


struct RectEvent :public EventArgs
{
	d2dw::CRectF Rect;
	RectEvent(CWnd* pWnd, d2dw::CRectF rect, BOOL* pHandled = nullptr) :
		EventArgs(pWnd, pHandled), Rect(rect){}
};

struct PaintEvent:public EventArgs
{
	using EventArgs::EventArgs;
};

struct MouseEvent:public EventArgs
{
	UINT Flags;
	CPoint PointInClient;
	MouseEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled), Flags(wParam), PointInClient((short)LOWORD(lParam), (short)HIWORD(lParam)){}
	virtual ~MouseEvent(){}
};

struct LButtonDownEvent :public MouseEvent
{
	LButtonDownEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr):
		MouseEvent(pWnd, wParam, lParam, pHandled){}
};

struct LButtonUpEvent :public MouseEvent
{
	LButtonUpEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr):
		MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonClkEvent :public MouseEvent
{
	LButtonClkEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonSnglClkEvent :public MouseEvent
{
	LButtonSnglClkEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonDblClkEvent :public MouseEvent
{
	LButtonDblClkEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonDblClkTimeExceedEvent :public MouseEvent
{
	LButtonDblClkTimeExceedEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonBeginDragEvent :public MouseEvent
{
	LButtonBeginDragEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct RButtonDownEvent :public MouseEvent
{
	RButtonDownEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct MouseMoveEvent :public MouseEvent
{
	MouseMoveEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct MouseLeaveEvent :public MouseEvent
{
	MouseLeaveEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};


struct MouseWheelEvent:public MouseEvent
{
	short Delta;
	MouseWheelEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled), Delta(GET_WHEEL_DELTA_WPARAM(wParam)){}
};

struct SetCursorEvent:public EventArgs
{
	UINT HitTest;
	SetCursorEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled), HitTest(LOWORD(lParam)){}
};

struct SetFocusEvent :public EventArgs
{
	HWND OldHWnd = nullptr;
	SetFocusEvent(CWnd* pWnd, WPARAM wParam = NULL, LPARAM lParam = NULL, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled),OldHWnd((HWND)wParam){}
};

struct ContextMenuEvent:public EventArgs
{
public:
	CPoint PointInClient;
	CPoint PointInScreen;
	ContextMenuEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled),
		PointInScreen((short)LOWORD(lParam), (short)HIWORD(lParam)),
		PointInClient(pWnd->ScreenToClient(PointInScreen)){}
};

struct CancelModeEvent :public EventArgs
{
public:
	CancelModeEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled){ }
};

struct CaptureChangedEvent :public EventArgs
{
public:
	HWND HWnd;
	CaptureChangedEvent(CWnd* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:EventArgs(pWnd, pHandled), HWnd(reinterpret_cast<HWND>(lParam)){ }
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
	virtual bool GetIsFocused()const { return false; }

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e) {}
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) {}
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) {}
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) {}

	virtual void OnRButtonDown(const RButtonDownEvent& e) {}


	virtual void OnMButtonDown(const MouseEvent& e){}//TODO
	virtual void OnMButtonUp(const MouseEvent& e){}//TODO

	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseEnter(const MouseEvent& e);//TODO
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e){}

	virtual void OnKeyDown(const KeyDownEvent& e) {}
	virtual void OnSysKeyDown(const SysKeyDownEvent& e){}
	virtual void OnChar(const CharEvent& e) {}
	virtual void OnContextMenu(const ContextMenuEvent& e){}
	virtual void OnSetFocus(const SetFocusEvent& e) {}
	virtual void OnSetCursor(const SetCursorEvent& e) {}
	virtual void OnKillFocus(const KillFocusEvent& e) {}


	virtual void OnPropertyChanged(const wchar_t* name){}
};
