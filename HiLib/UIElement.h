#pragma once
#include "MyWnd.h"
#include "MyPoint.h"
#include "Direct2DWrite.h"

class CD2DWWindow;
class CD2DWControl;

enum class Visibility
{
	Disabled,//Not visble, Not scrollable by any method
	Auto,//Depends on page size, content size
	Hidden,//Not Visible, but scrollable by mouse wheel
	Visible,//Force visible
};

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

struct Event
{
public:
	CD2DWWindow* WndPtr;
	BOOL* HandledPtr;
	Event(CD2DWWindow* pWnd = nullptr, BOOL* pHandled = nullptr):WndPtr(pWnd), HandledPtr(pHandled){}
};

class CCell;

struct BeginEditEvent :public Event
{
	CCell* CellPtr;
	BeginEditEvent(CD2DWWindow* pWnd, CCell* pCell)
		:Event(pWnd), CellPtr(pCell) {}
};

struct EndEditEvent :public Event
{
	EndEditEvent(CD2DWWindow* pWnd)
		:Event() {}
};


struct CreateEvt :public Event
{
	CRect Rect;
	CRectF RectF;
	CD2DWControl* ParentPtr;
	CreateEvt(CD2DWWindow* pWnd, CD2DWControl* pParent, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr);
	CreateEvt(CD2DWWindow* pWnd, CD2DWControl* pParent, CRectF rect, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled), ParentPtr(pParent), RectF(rect){}
};

struct DestroyEvent :public Event
{
	DestroyEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled){}
	DestroyEvent(CD2DWWindow* pWnd, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled){}
};


struct CloseEvent :public Event
{
	CloseEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled) {}
};

struct ClosingEvent :public CloseEvent
{
	std::unique_ptr<bool> CancelPtr;
	ClosingEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:CloseEvent(pWnd, wParam, lParam, pHandled), CancelPtr(std::make_unique<bool>(false)) {}
};

struct KillFocusEvent :public Event
{
	KillFocusEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled){}
};

struct KeyEvent:public Event
{
	UINT Char;
	UINT RepeatCount;
	UINT Flags;
	KeyEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled), Char(wParam), RepeatCount(lParam & 0xFF), Flags(lParam >> 16 & 0xFF) {}
};

struct CharEvent :public KeyEvent
{
	CharEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:KeyEvent(pWnd, wParam, lParam, pHandled){}
};

struct KeyDownEvent :public KeyEvent
{
	KeyDownEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:KeyEvent(pWnd, wParam, lParam, pHandled) {}
};

struct KeyUpEvent :public KeyEvent
{
	KeyUpEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:KeyEvent(pWnd, wParam, lParam, pHandled) {}
};


struct SysKeyDownEvent :public KeyDownEvent
{
	using KeyDownEvent::KeyDownEvent;
};


struct RectEvent :public Event
{
	CRectF Rect;
	RectEvent(CD2DWWindow* pWnd, CRectF rect, BOOL* pHandled = nullptr) :
		Event(pWnd, pHandled), Rect(rect){}
};

struct PaintEvent:public Event
{
	using Event::Event;
};

struct MouseEvent:public Event
{
	UINT Flags;
	CPoint PointInClient;
	CPointF PointInWnd;//Since member variable initialized in order of declarelation, order should be PointInCliend->PointInWnd
	MouseEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr);
	virtual ~MouseEvent(){}
};

struct LButtonDownEvent :public MouseEvent
{
	LButtonDownEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr):
		MouseEvent(pWnd, wParam, lParam, pHandled){}
};

struct LButtonUpEvent :public MouseEvent
{
	LButtonUpEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr):
		MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonClkEvent :public MouseEvent
{
	LButtonClkEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonSnglClkEvent :public MouseEvent
{
	LButtonSnglClkEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonDblClkEvent :public MouseEvent
{
	LButtonDblClkEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonDblClkTimeExceedEvent :public MouseEvent
{
	LButtonDblClkTimeExceedEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonBeginDragEvent :public MouseEvent
{
	LButtonBeginDragEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct LButtonEndDragEvent :public MouseEvent
{
	LButtonEndDragEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct RButtonDownEvent :public MouseEvent
{
	RButtonDownEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct MButtonDownEvent :public MouseEvent
{
	MButtonDownEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr):
		MouseEvent(pWnd, wParam, lParam, pHandled){}
};

struct MButtonUpEvent :public MouseEvent
{
	MButtonUpEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr):
		MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct MouseMoveEvent :public MouseEvent
{
	MouseMoveEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};

struct MouseEnterEvent :public MouseEvent
{
	MouseEnterEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};


struct MouseLeaveEvent :public MouseEvent
{
	MouseLeaveEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:MouseEvent(pWnd, wParam, lParam, pHandled) {}
};


struct MouseWheelEvent:public Event
{
	CPoint PointInScreen;
	CPoint PointInClient;
	CPointF PointInWnd;

	short Delta;
	MouseWheelEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr);
};

struct SetCursorEvent:public Event
{
	UINT HitTest;
	CPointF PointInWnd;
	SetCursorEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr);
	SetCursorEvent(CD2DWWindow* pWnd, UINT hittest, BOOL* pHandled = nullptr);
};

struct SetFocusEvent :public Event
{
	HWND OldHWnd = nullptr;
	SetFocusEvent(CD2DWWindow* pWnd, WPARAM wParam = NULL, LPARAM lParam = NULL, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled),OldHWnd((HWND)wParam){}
};

struct ContextMenuEvent:public Event
{
public:
	CPoint PointInScreen;
	CPoint PointInClient;
	CPointF PointInWnd;
	ContextMenuEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr);
};

struct CommandEvent :public Event
{
	UINT ID;
	UINT NotifyCode;
	HWND HWndControl;
	CommandEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
	:Event(pWnd, pHandled),ID(wParam & 0xFFFF), NotifyCode((wParam>>16) & 0xFFFF), HWndControl((HWND)lParam){}
};

struct CancelModeEvent :public Event
{
public:
	CancelModeEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled){ }
};

struct CaptureChangedEvent :public Event
{
public:
	HWND HWnd;
	CaptureChangedEvent(CD2DWWindow* pWnd, WPARAM wParam, LPARAM lParam, BOOL* pHandled = nullptr)
		:Event(pWnd, pHandled), HWnd(reinterpret_cast<HWND>(lParam)){ }
};



class CUIElement:public std::enable_shared_from_this<CUIElement>
{
protected:
	UIElementState::Type m_state;

public:
	CUIElement()
		:m_state(UIElementState::Normal){}
	virtual ~CUIElement() = default;

	/*********/
	/* state */
	/*********/
	virtual UIElementState::Type GetState()const{return m_state;}
	virtual void SetState(const UIElementState::Type& state);
	



	/*********/
	/* event */
	/*********/
	virtual void OnCreate(const CreateEvt& e) {}
	virtual void OnDestroy(const DestroyEvent& e) {}
	virtual void OnPaint(const PaintEvent& e) {}
	virtual void OnClosing(const ClosingEvent& e) {}
	virtual void OnClose(const CloseEvent& e) {}
	virtual void OnCommand(const CommandEvent& e) {}
	virtual void OnRect(const RectEvent& e) {}

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e) {}
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) {}
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) {}
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) {}
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) {}

	virtual void OnRButtonDown(const RButtonDownEvent& e) {}

	virtual void OnMButtonDown(const MButtonDownEvent& e){}
	virtual void OnMButtonUp(const MButtonUpEvent& e){}

	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseEnter(const MouseEnterEvent& e);//TODO
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e){}

	virtual void OnCaptureChanged(const CaptureChangedEvent& e) {}

	virtual void OnKeyDown(const KeyDownEvent& e) {}
	virtual void OnKeyUp(const KeyUpEvent& e) {}
	virtual void OnSysKeyDown(const SysKeyDownEvent& e){}
	virtual void OnChar(const CharEvent& e) {}
	virtual void OnContextMenu(const ContextMenuEvent& e){}
	virtual void OnSetCursor(const SetCursorEvent& e) {}

	virtual void OnSetFocus(const SetFocusEvent& e) {}
	virtual void OnKillFocus(const KillFocusEvent& e) {}

	virtual void OnWndSetFocus(const SetFocusEvent& e) {};
	virtual void OnWndKillFocus(const KillFocusEvent& e) {};

	virtual void OnPropertyChanged(const wchar_t* name){}
	
	virtual CRectF GetRectInWnd() const = 0;

	void Update() {}

};

