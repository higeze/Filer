#pragma once
#include "UIElement.h"

class CGridView;
class CCell;

class ICeller
{
public:
	virtual void OnLButtonDown(CGridView* pSheet, const LButtonDownEvent& e) = 0;
	virtual void OnLButtonUp(CGridView* pSheet, const LButtonUpEvent& e) = 0;
	virtual void OnLButtonClk(CGridView* pSheet, const LButtonClkEvent& e) = 0;
	virtual void OnLButtonSnglClk(CGridView* pSheet, const LButtonSnglClkEvent& e) = 0;
	virtual void OnLButtonDblClk(CGridView* pSheet, const LButtonDblClkEvent& e) = 0;
	virtual void OnLButtonBeginDrag(CGridView* pSheet, const LButtonBeginDragEvent& e) = 0;
	virtual void OnMouseMove(CGridView* pSheet, const MouseMoveEvent& e) = 0;
	virtual void OnMouseLeave(CGridView* pSheet, const MouseLeaveEvent& e) = 0;
	virtual void OnContextMenu(CGridView* pSheet, const ContextMenuEvent& e) = 0;
	//virtual void OnSetFocus(CGridView* pSheet, const EventArgs& e) = 0;
	//virtual void OnKillFocus(CGridView* pSheet, const EventArgs& e) = 0;
	virtual void OnSetCursor(CGridView* pSheet, const SetCursorEvent& e) = 0;
	virtual void OnKeyDown(CGridView* pSheet, const KeyDownEvent& e) = 0;
	virtual void OnKeyTraceDown(CGridView* pSheet, const KeyTraceDownEvent& e) = 0;
	virtual void OnChar(CGridView* pSheet, const CharEvent& e) = 0;
	virtual void OnImeStartComposition(CGridView* pSheet, const ImeStartCompositionEvent& e) = 0;


	virtual void OnClear() = 0;
	virtual void Clear() = 0;
};

class CCeller :public ICeller
{
private:
	std::shared_ptr<CCell> m_cellUnderMouse;
public:
	virtual void OnLButtonDown(CGridView* pSheet, const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(CGridView* pSheet, const LButtonUpEvent& e) override;
	virtual void OnLButtonClk(CGridView* pSheet, const LButtonClkEvent& e) override;
	virtual void OnLButtonSnglClk(CGridView* pSheet, const LButtonSnglClkEvent& e) override;
	virtual void OnLButtonDblClk(CGridView* pSheet, const LButtonDblClkEvent& e) override;
	virtual void OnLButtonBeginDrag(CGridView* pSheet, const LButtonBeginDragEvent& e) override;
	virtual void OnMouseMove(CGridView* pSheet, const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(CGridView* pSheet, const MouseLeaveEvent& e) override;
	virtual void OnContextMenu(CGridView* pSheet, const ContextMenuEvent& e) override;
	//virtual void OnSetFocus(CGridView* pSheet, const EventArgs& e);
	//virtual void OnKillFocus(CGridView* pSheet, const EventArgs& e);
	virtual void OnSetCursor(CGridView* pSheet, const SetCursorEvent& e) override;
	virtual void OnKeyDown(CGridView* pSheet, const KeyDownEvent& e) override;
	virtual void OnKeyTraceDown(CGridView* pSheet, const KeyTraceDownEvent& e) override;
	virtual void OnChar(CGridView* pSheet, const CharEvent& e) override;;
	virtual void OnImeStartComposition(CGridView* pSheet, const ImeStartCompositionEvent& e) override;

	virtual void OnClear() override;
	virtual void Clear() override;
};