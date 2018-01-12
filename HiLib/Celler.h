#pragma once

class CSheet;
class CCell;
struct EventArgs;
struct MouseEventArgs;
struct SetCursorEventArgs;
struct ContextMenuEventArgs;
struct KeyEventArgs;

class ICeller
{
public:
	virtual void OnLButtonDown(CSheet* pSheet, const MouseEventArgs& e) = 0;
	virtual void OnLButtonUp(CSheet* pSheet, const MouseEventArgs& e) = 0;
	virtual void OnLButtonSnglClk(CSheet* pSheet, const MouseEventArgs& e) = 0;
	virtual void OnLButtonDblClk(CSheet* pSheet, const MouseEventArgs& e) = 0;
	virtual void OnMouseMove(CSheet* pSheet, const MouseEventArgs& e) = 0;
	virtual void OnMouseLeave(CSheet* pSheet, const MouseEventArgs& e) = 0;
	virtual void OnContextMenu(CSheet* pSheet, const ContextMenuEventArgs& e) = 0;
	//virtual void OnSetFocus(CSheet* pSheet, const EventArgs& e) = 0;
	//virtual void OnKillFocus(CSheet* pSheet, const EventArgs& e) = 0;
	virtual void OnSetCursor(CSheet* pSheet, const SetCursorEventArgs& e) = 0;
	virtual void OnKeyDown(CSheet* pSheet, const KeyEventArgs& e) = 0;
};

class CCeller :public ICeller
{
private:
	std::shared_ptr<CCell> m_cellUnderMouse;
public:
	virtual void OnLButtonDown(CSheet* pSheet, const MouseEventArgs& e);
	virtual void OnLButtonUp(CSheet* pSheet, const MouseEventArgs& e);
	virtual void OnLButtonSnglClk(CSheet* pSheet, const MouseEventArgs& e);
	virtual void OnLButtonDblClk(CSheet* pSheet, const MouseEventArgs& e);
	virtual void OnMouseMove(CSheet* pSheet, const MouseEventArgs& e);
	virtual void OnMouseLeave(CSheet* pSheet, const MouseEventArgs& e);
	virtual void OnContextMenu(CSheet* pSheet, const ContextMenuEventArgs& e);
	//virtual void OnSetFocus(CSheet* pSheet, const EventArgs& e);
	//virtual void OnKillFocus(CSheet* pSheet, const EventArgs& e);
	virtual void OnSetCursor(CSheet* pSheet, const SetCursorEventArgs& e);
	virtual void OnKeyDown(CSheet* pSheet, const KeyEventArgs& e);
};