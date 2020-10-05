#pragma once
#include "SheetStateMachine.h"
#include <boost/sml.hpp>

class CTextBox;

class CTextBoxStateMachine :public IStateMachine
{
private:
	struct Machine;
	std::unique_ptr<boost::sml::sm<Machine>> m_pMachine;
public:
	CTextBoxStateMachine(CTextBox* pTextbox);
	~CTextBoxStateMachine();

	virtual void process_event(const PaintEvent& e) override;
	virtual void process_event(const RButtonDownEvent& e) override;
	virtual void process_event(const LButtonDownEvent& e) override;
	virtual void process_event(const LButtonUpEvent& e) override;
	virtual void process_event(const LButtonClkEvent& e) override;
	virtual void process_event(const LButtonSnglClkEvent& e) override;
	virtual void process_event(const LButtonDblClkEvent& e) override;
	virtual void process_event(const LButtonBeginDragEvent& e) override;
	virtual void process_event(const LButtonEndDragEvent& e) override;
	virtual void process_event(const ContextMenuEvent& e) override;
	virtual void process_event(const MouseMoveEvent& e) override;
	virtual void process_event(const MouseLeaveEvent& e) override;
	virtual void process_event(const SetCursorEvent& e) override;
	virtual void process_event(const SetFocusEvent& e) override;
	virtual void process_event(const KillFocusEvent& e) override;
	virtual void process_event(const KeyDownEvent& e) override;
	virtual void process_event(const CharEvent& e) override;
	virtual void process_event(const BeginEditEvent& e) override {}
	virtual void process_event(const EndEditEvent& e) override {}
};
