#pragma once
#include "UIElement.h"
#include <boost/sml.hpp>

class CSheet;

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
	virtual void process_event(const RButtonDownEvent& e) = 0;
	virtual void process_event(const ContextMenuEvent& e) = 0;
	virtual void process_event(const MouseMoveEvent& e) = 0;
	virtual void process_event(const MouseLeaveEvent& e) = 0;
	virtual void process_event(const SetCursorEvent& e) = 0;
	virtual void process_event(const SetFocusEvent& e) = 0;
	virtual void process_event(const KillFocusEvent& e) = 0;
	virtual void process_event(const KeyDownEvent& e) = 0;
	virtual void process_event(const CharEvent& e) = 0;
	virtual void process_event(const BeginEditEvent& e) = 0;
};

class CSheetStateMachine:public IStateMachine
{
private:
	struct Machine;
	std::unique_ptr<boost::sml::sm<Machine>> m_pMachine;
public:
	CSheetStateMachine(CSheet* pSheet);
	~CSheetStateMachine();

	virtual void process_event(const PaintEvent& e) override;
	virtual void process_event(const RButtonDownEvent& e) override;
	virtual void process_event(const LButtonDownEvent& e) override;
	virtual void process_event(const LButtonUpEvent& e) override;
	virtual void process_event(const LButtonClkEvent& e) override;
	virtual void process_event(const LButtonSnglClkEvent& e) override;
	virtual void process_event(const LButtonDblClkEvent& e) override;
	virtual void process_event(const LButtonBeginDragEvent& e) override;
	virtual void process_event(const ContextMenuEvent& e) override;
	virtual void process_event(const MouseMoveEvent& e) override;
	virtual void process_event(const MouseLeaveEvent& e) override;
	virtual void process_event(const SetCursorEvent& e) override;
	virtual void process_event(const SetFocusEvent& e) override;
	virtual void process_event(const KillFocusEvent& e) override;
	virtual void process_event(const KeyDownEvent& e) override;
	virtual void process_event(const CharEvent& e) override;

	virtual void process_event(const BeginEditEvent& e) override;
};

