#pragma once
#include "SheetStateMachine.h"
#include <boost/sml.hpp>
#include <queue>

class CGridView;

struct my_logger {
  template <class SM, class TEvent>
  void log_process_event(const TEvent&) {
    ::OutputDebugStringA(std::format("[{}][process_event] {}\r\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TEvent>()).c_str());
  }

  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard&, const TEvent&, bool result) {
    ::OutputDebugStringA(std::format("[{}][guard] {} {} {}\r\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TGuard>(),
           boost::sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]")).c_str());
  }

  template <class SM, class TAction, class TEvent>
  void log_action(const TAction&, const TEvent&) {
    ::OutputDebugStringA(std::format("[{}][action] {} {}\r\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TAction>(),
           boost::sml::aux::get_type_name<TEvent>()).c_str());
  }

  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {
    ::OutputDebugStringA(std::format("[{}][transition] {} -> %s\n", boost::sml::aux::get_type_name<SM>(), src.c_str(), dst.c_str()).c_str());
  }
};

class CGridStateMachine :public IStateMachine
{
private:
	//my_logger m_logger;
	struct Machine;
	//std::unique_ptr<boost::sml::sm<Machine, boost::sml::logger<my_logger>>> m_pMachine;
	std::unique_ptr<boost::sml::sm<Machine, boost::sml::process_queue<std::queue>>> m_pMachine;
	//std::unique_ptr<boost::sml::sm<Machine>> m_pMachine;
public:
	CGridStateMachine(CGridView* pGrid);
	~CGridStateMachine();

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
	virtual void process_event(const KeyUpEvent& e) override {}
	virtual void process_event(const KeyTraceDownEvent& e) override;
	virtual void process_event(const KeyTraceUpEvent& e) override {}
	virtual void process_event(const CharEvent& e) override;
	virtual void process_event(const ImeStartCompositionEvent& e) override;
	virtual void process_event(const BeginEditEvent& e) override;
	virtual void process_event(const EndEditEvent& e) override;
};
