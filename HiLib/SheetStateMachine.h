#pragma
#include "UIElement.h"
#include <memory>
#include <boost\msm\front\state_machine_def.hpp>

//Pre-declare
class CSheet;

struct CSheetStateMachine
{
	CSheetStateMachine(CSheet* pSheet);
	~CSheetStateMachine();

	struct Impl;
	std::unique_ptr<Impl> pImpl;

	void LButtonDown(const LButtonDownEvent& e);
	void LButtonUp(const LButtonUpEvent& e);
	void LButtonClk(const LButtonClkEvent& e);
	void LButtonSnglClk(const LButtonSnglClkEvent& e);
	void LButtonDblClk(const LButtonDblClkEvent& e);
	void LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void RButtonDown(const RButtonDownEvent& e);
	void ContextMenu(const ContextMenuEvent& e);
	void MouseMove(const MouseMoveEvent& e);
	void MouseLeave(const MouseLeaveEvent& e);
	void SetCursor(const SetCursorEvent& e);
	void KeyDown(const KeyDownEvent& e);
};