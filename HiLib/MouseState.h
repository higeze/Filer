#pragma once
#include "MyPoint.h"
class CSheet;
struct MouseEventArgs;
struct SetCursorEventArgs;

class IMouseState
{
protected:
	static bool m_isDblClkTimeExceed;
public:
	IMouseState() {}
	virtual ~IMouseState() {}

	virtual void Entry(CSheet* pSheet, MouseEventArgs& e);
	virtual void Exit(CSheet* pSheet, MouseEventArgs& e) { std::cout << "IMouseState::Exit" << std::endl; };
	virtual IMouseState* ChangeState(CSheet* pSheet, IMouseState* pMouseState, MouseEventArgs& e);
	virtual IMouseState* KeepState();
	virtual IMouseState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e) = 0;
	virtual IMouseState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e) = 0;
	virtual IMouseState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e) = 0;
	virtual IMouseState* OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e) = 0;
	virtual IMouseState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
};

class CDefaultMouseState :public IMouseState
{
public:
	CDefaultMouseState() {}
	virtual ~CDefaultMouseState() {}
	static IMouseState* State();
	virtual IMouseState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)override;
};

class CDownedMouseState :public IMouseState
{
public:
	CDownedMouseState() {}
	virtual ~CDownedMouseState() {}
	static IMouseState* State();
	virtual void Entry(CSheet* pSheet, MouseEventArgs& e) override;
	virtual IMouseState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)override;
};

class CUppedMouseState :public IMouseState
{
public:
	CUppedMouseState() {}
	virtual ~CUppedMouseState() {}
	static IMouseState* State();
	virtual void Entry(CSheet* pSheet, MouseEventArgs& e) override;
	virtual IMouseState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)override;
};

class CDblClkedMouseState :public IMouseState
{
public:
	CDblClkedMouseState() {}
	virtual ~CDblClkedMouseState() {}
	static IMouseState* State();
	virtual IMouseState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)override;
	virtual IMouseState* OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)override;
};
