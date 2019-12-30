#pragma once
#include "UIElement.h"
#include <boost/sml.hpp>

//Pre-declare
class CGridView;

class CMouseStateMachine
{
private:
	struct Machine;
	std::unique_ptr<boost::sml::sm<Machine>> m_pMachine;
public:
	CMouseStateMachine(CGridView* pGrid);
	~CMouseStateMachine();

	void process_event(const LButtonDownEvent& e);
	void process_event(const LButtonUpEvent& e);
	void process_event(const LButtonDblClkEvent& e);
	void process_event(const LButtonDblClkTimeExceedEvent& e);
	void process_event(const MouseLeaveEvent& e);
};