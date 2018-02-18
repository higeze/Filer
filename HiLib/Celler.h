#pragma once
#include "UIElement.h"
class CSheet;
class CCell;

class ICeller
{
public:
	virtual void OnLButtonDown(CSheet* pSheet, const LButtonDownEvent& e) = 0;
	virtual void OnLButtonUp(CSheet* pSheet, const LButtonUpEvent& e) = 0;
	virtual void OnLButtonClk(CSheet* pSheet, const LButtonClkEvent& e) = 0;
	virtual void OnLButtonSnglClk(CSheet* pSheet, const LButtonSnglClkEvent& e) = 0;
	virtual void OnLButtonDblClk(CSheet* pSheet, const LButtonDblClkEvent& e) = 0;
	virtual void OnMouseMove(CSheet* pSheet, const MouseMoveEvent& e) = 0;
	virtual void OnMouseLeave(CSheet* pSheet, const MouseLeaveEvent& e) = 0;
	virtual void OnContextMenu(CSheet* pSheet, const ContextMenuEvent& e) = 0;
	//virtual void OnSetFocus(CSheet* pSheet, const EventArgs& e) = 0;
	//virtual void OnKillFocus(CSheet* pSheet, const EventArgs& e) = 0;
	virtual void OnSetCursor(CSheet* pSheet, const SetCursorEvent& e) = 0;
	virtual void OnKeyDown(CSheet* pSheet, const KeyDownEvent& e) = 0;
};

class CCeller :public ICeller
{
private:
	std::shared_ptr<CCell> m_cellUnderMouse;
public:
	virtual void OnLButtonDown(CSheet* pSheet, const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(CSheet* pSheet, const LButtonUpEvent& e) override;
	virtual void OnLButtonClk(CSheet* pSheet, const LButtonClkEvent& e) override;
	virtual void OnLButtonSnglClk(CSheet* pSheet, const LButtonSnglClkEvent& e) override;
	virtual void OnLButtonDblClk(CSheet* pSheet, const LButtonDblClkEvent& e) override;
	virtual void OnMouseMove(CSheet* pSheet, const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(CSheet* pSheet, const MouseLeaveEvent& e) override;
	virtual void OnContextMenu(CSheet* pSheet, const ContextMenuEvent& e) override;
	//virtual void OnSetFocus(CSheet* pSheet, const EventArgs& e);
	//virtual void OnKillFocus(CSheet* pSheet, const EventArgs& e);
	virtual void OnSetCursor(CSheet* pSheet, const SetCursorEvent& e) override;
	virtual void OnKeyDown(CSheet* pSheet, const KeyDownEvent& e) override;
};