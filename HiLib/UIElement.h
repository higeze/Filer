#pragma once
#include "MyRect.h"
#include "MySize.h"
#include "MyDC.h"
#include "MyFont.h"
#include "MyPen.h"
#include "MyWnd.h"

namespace UIElementState
{
	enum Type{
		None,
		Normal,
		Hot,
		Pressed,
		PressedLeave,
		Disabled
	};
}

struct EventArgs
{
public:
	CWnd* WindowPtr;
	EventArgs(CWnd* pWnd = nullptr):WindowPtr(pWnd){}
};

struct KeyEventArgs:public EventArgs
{
	UINT Char;
	UINT RepeatCount;
	UINT Flags;
	KeyEventArgs(UINT ch,UINT uRepCnt,UINT uFlags)
		:Char(ch),RepeatCount(uRepCnt),Flags(uFlags){}
};

struct KeyDownEvent :public KeyEventArgs
{
	KeyDownEvent(UINT ch, UINT uRepCnt, UINT uFlags):KeyEventArgs(ch, uRepCnt, uFlags){}
};

struct PaintEvent:public EventArgs
{
	CDC* DCPtr;
	PaintEvent(CWnd* pWnd, CDC* pDC)
		:EventArgs(pWnd),DCPtr(pDC){}
	PaintEvent(CDC* pDC)
		:EventArgs(),DCPtr(pDC){}
};

//struct OGLPaintEventArgs:public PaintEventArgs
//{
//	CDC* DCPtr;
//	COGLRenderer* OGLRendererPtr;
//	OGLPaintEventArgs(COGLRenderer* pOGLRenderer, CDC* pDC)
//		:PaintEventArgs(pDC),OGLRendererPtr(pOGLRenderer){}
//};

struct MouseEvent:public EventArgs
{
	UINT Flags;
	CPoint Point;
	MouseEvent(UINT uFlags,CPoint pt)
		:Flags(uFlags),Point(pt){}
	virtual ~MouseEvent(){}
};

struct LButtonDownEvent :public MouseEvent
{
	LButtonDownEvent(UINT uFlags, CPoint pt):MouseEvent(uFlags, pt){}
};

struct LButtonUpEvent :public MouseEvent
{
	LButtonUpEvent(UINT uFlags, CPoint pt):MouseEvent(uFlags, pt) {}
};

struct LButtonClkEvent :public MouseEvent
{
	LButtonClkEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};

struct LButtonSnglClkEvent :public MouseEvent
{
	LButtonSnglClkEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};

struct LButtonDblClkEvent :public MouseEvent
{
	LButtonDblClkEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};

struct LButtonDblClkTimeExceedEvent :public MouseEvent
{
	LButtonDblClkTimeExceedEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};

struct LButtonBeginDragEvent :public MouseEvent
{
	LButtonBeginDragEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};

struct RButtonDownEvent :public MouseEvent
{
	RButtonDownEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};

struct MouseMoveEvent :public MouseEvent
{
	MouseMoveEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};

struct MouseLeaveEvent :public MouseEvent
{
	MouseLeaveEvent(UINT uFlags, CPoint pt) :MouseEvent(uFlags, pt) {}
};


struct MouseWheelEvent:public MouseEvent
{
	short Delta;
	MouseWheelEvent(UINT uFlags, short zDelta, CPoint pt)
		:MouseEvent(uFlags, pt),Delta(zDelta){}
	virtual ~MouseWheelEvent(){}
};

struct SetCursorEvent:public EventArgs
{
	HWND HWnd;
	UINT HitTest;
	BOOL& Handled;
	SetCursorEvent(HWND HWnd, UINT nHitTest, BOOL& Handled)
		:HWnd(HWnd), HitTest(nHitTest), Handled(Handled){}
	virtual ~SetCursorEvent(){}
};

struct SetFocusEvent :public EventArgs
{
	SetFocusEvent():EventArgs(){}
};

struct KillFocusEvent :public EventArgs
{
	KillFocusEvent() :EventArgs() {}
};


struct ContextMenuEvent:public EventArgs
{
public:
	CPoint Point;
	ContextMenuEvent(CWnd* pWnd, CPoint pt)
		:EventArgs(pWnd),Point(pt){}
	virtual ~ContextMenuEvent(){}
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
	virtual void SetState(const UIElementState::Type& state)
	{
		if(m_state!=state){
			m_state=state;
		}
	}

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnMButtonDown(const MouseEvent& e){}//TODO
	virtual void OnMButtonUp(const MouseEvent& e){}//TODO
	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseEnter(const MouseEvent& e);//TODO
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e){}
	virtual void OnLButtonClk(const LButtonClkEvent& e){}
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e){}
	virtual void OnKeyDown(const KeyDownEvent& e){}
	virtual void OnContextMenu(const ContextMenuEvent& e){}
};
