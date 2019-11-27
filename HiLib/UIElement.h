#pragma once
#include "MyWnd.h"
#include "MyPoint.h"
#include "Direct2DWrite.h"

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
	KeyEventArgs(d2dw::CDirect2DWrite* pDirect, WPARAM wParam, LPARAM lParam) :
		Char(wParam), RepeatCount(lParam & 0xFF), Flags(lParam >> 16 & 0xFF) {}
};

struct CharEvent :public KeyEventArgs
{
	CharEvent(d2dw::CDirect2DWrite* pDirect, WPARAM wParam, LPARAM lParam)
		:KeyEventArgs(pDirect, wParam, lParam){}
	CharEvent(UINT ch, UINT uRepCnt, UINT uFlags) :KeyEventArgs(ch, uRepCnt, uFlags) {}
};


struct KeyDownEvent :public KeyEventArgs
{
	KeyDownEvent(d2dw::CDirect2DWrite* pDirect, WPARAM wParam, LPARAM lParam)
		:KeyEventArgs(pDirect, wParam, lParam) {}
	KeyDownEvent(UINT ch, UINT uRepCnt, UINT uFlags):KeyEventArgs(ch, uRepCnt, uFlags){}
};

struct PaintEvent:public EventArgs
{
	d2dw::CDirect2DWrite& Direct;
	d2dw::CDirect2DWrite* DirectPtr;

	//CDC* DCPtr;
	PaintEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct)
		:EventArgs(pWnd), Direct(direct){}
	PaintEvent(d2dw::CDirect2DWrite& direct)
		:EventArgs(), Direct(direct){}
	PaintEvent(d2dw::CDirect2DWrite* pDirect)
		:EventArgs(), DirectPtr(pDirect), Direct(*pDirect){}
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
	d2dw::CDirect2DWrite& Direct;
	d2dw::CDirect2DWrite* DirectPtr;
	UINT Flags;
	CPoint Point;
	MouseEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt)
		:EventArgs(pWnd), Direct(direct), Flags(uFlags),Point(pt){}
	MouseEvent(d2dw::CDirect2DWrite* pDirect, WPARAM wParam, LPARAM lParam)
		:EventArgs(nullptr), DirectPtr(pDirect), Direct(*pDirect), Flags((UINT)wParam), Point((short)LOWORD(lParam), (short)HIWORD(lParam)){}
	virtual ~MouseEvent(){}
};

struct LButtonDownEvent :public MouseEvent
{
	LButtonDownEvent(d2dw::CDirect2DWrite* pDirect, WPARAM wParam, LPARAM lParam):
		MouseEvent(pDirect, wParam, lParam){}
	LButtonDownEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt):MouseEvent(pWnd, direct, uFlags, pt){}
};

struct LButtonUpEvent :public MouseEvent
{
	LButtonUpEvent(d2dw::CDirect2DWrite* pDirect, WPARAM wParam, LPARAM lParam) :
		MouseEvent(pDirect, wParam, lParam) {}
	LButtonUpEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt):MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct LButtonClkEvent :public MouseEvent
{
	LButtonClkEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct LButtonSnglClkEvent :public MouseEvent
{
	LButtonSnglClkEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct LButtonDblClkEvent :public MouseEvent
{
	LButtonDblClkEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct LButtonDblClkTimeExceedEvent :public MouseEvent
{
	LButtonDblClkTimeExceedEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct LButtonBeginDragEvent :public MouseEvent
{
	LButtonBeginDragEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct RButtonDownEvent :public MouseEvent
{
	RButtonDownEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct MouseMoveEvent :public MouseEvent
{
	MouseMoveEvent(d2dw::CDirect2DWrite* pDirect, LPARAM lParam, WPARAM wParam) :
		MouseEvent(pDirect, lParam, wParam) {}

	MouseMoveEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};

struct MouseLeaveEvent :public MouseEvent
{
	MouseLeaveEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, CPoint pt) :MouseEvent(pWnd, direct, uFlags, pt) {}
};


struct MouseWheelEvent:public MouseEvent
{
	short Delta;
	MouseWheelEvent(CWnd* pWnd, d2dw::CDirect2DWrite& direct, UINT uFlags, short zDelta, CPoint pt)
		:MouseEvent(pWnd, direct, uFlags, pt),Delta(zDelta){}
	virtual ~MouseWheelEvent(){}
};

struct SetCursorEvent:public EventArgs
{
	UINT HitTest;
	BOOL& Handled;
	SetCursorEvent(CWnd* pWnd, UINT nHitTest, BOOL& Handled)
		:EventArgs(pWnd), HitTest(nHitTest), Handled(Handled){}
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
