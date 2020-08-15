#pragma once
#include "UIElement.h"
#include <boost/sml.hpp>

//Pre-declare
class CUIElement;

class CMouseStateMachine
{
private:
	struct Machine;
	std::unique_ptr<boost::sml::sm<Machine>> m_pMachine;
public:
	CMouseStateMachine(CUIElement* pGrid);
	~CMouseStateMachine();

	void process_event(const LButtonDownEvent& e);
	void process_event(const LButtonUpEvent& e);
	void process_event(const LButtonDblClkEvent& e);
	void process_event(const LButtonDblClkTimeExceedEvent& e);
	void process_event(const RButtonDownEvent& e);
	void process_event(const MouseMoveEvent& e);
	void process_event(const MouseLeaveEvent& e);
	void process_event(const MouseWheelEvent& e);
	void process_event(const CharEvent& e);
	void process_event(const KeyDownEvent& e);
	void process_event(const CancelModeEvent& e);
	void process_event(const CaptureChangedEvent& e);
};