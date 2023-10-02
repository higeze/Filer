#pragma once
#include "UIElement.h"

class IStateMachine
{
public:
	virtual void process_event(const PaintEvent& e) = 0;
	virtual void process_event(const LButtonDownEvent& e) = 0;
	virtual void process_event(const LButtonUpEvent& e) = 0;
	virtual void process_event(const LButtonClkEvent& e) = 0;
	virtual void process_event(const LButtonSnglClkEvent& e) = 0;
	virtual void process_event(const LButtonDblClkEvent& e) = 0;
	virtual void process_event(const LButtonBeginDragEvent& e) = 0;
	virtual void process_event(const LButtonEndDragEvent& e) = 0;
	virtual void process_event(const RButtonDownEvent& e) = 0;
	virtual void process_event(const ContextMenuEvent& e) = 0;
	virtual void process_event(const MouseMoveEvent& e) = 0;
	virtual void process_event(const MouseLeaveEvent& e) = 0;
	virtual void process_event(const SetCursorEvent& e) = 0;
	virtual void process_event(const SetFocusEvent& e) = 0;
	virtual void process_event(const KillFocusEvent& e) = 0;
	virtual void process_event(const KeyDownEvent& e) = 0;
	virtual void process_event(const KeyUpEvent& e) = 0;
	virtual void process_event(const CharEvent& e) = 0;
	virtual void process_event(const ImeStartCompositionEvent& e) = 0;
	virtual void process_event(const BeginEditEvent& e) = 0;
	virtual void process_event(const EndEditEvent& e) = 0;
};
