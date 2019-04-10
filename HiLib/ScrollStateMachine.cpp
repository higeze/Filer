#include "ScrollStateMachine.h"
#include "Scroll.h"
#include "SheetEventArgs.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

using namespace boost::msm::front;
using namespace d2dw;

struct CScrollStateMachine::Impl :state_machine_def<CScrollStateMachine::Impl>
{
	//Event
	struct Exception {};

	//State
	struct NormalState :state<> {};
	struct DragState :state<> {};

	//Machine

	struct Machine_ :state_machine_def<CScrollStateMachine::Impl::Machine_>
	{
		//Normal
		template<class Event>
		void Action_Normal_LButtonDown(Event const & e)
		{
		}

		template<class Event>
		void Action_Normal_LButtonUp(Event const & e)
		{
		}

		template<class Event>
		void Action_Normal_MouseMove(Event const & e)
		{
		}

		template<class Event>
		bool Guard_Normal_LButtonDown(Event const & e)
		{
			return true;
		}

		template<class Event>
		void Action_Drag_LButtonUp(Event const & e)
		{
		}

		template<class Event>
		void Action_Drag_MouseMove(Event const & e)
		{
		}

		struct transition_table :boost::mpl::vector <
			//     Start      Event             Target       Action                                Guard
			row<NormalState,  LButtonDownEvent, DragState,   &Machine_::Action_Normal_LButtonDown, &Machine_::Guard_Normal_LButtonDown>,
			_row<NormalState, Exception,        NormalState>,

			a_row<DragState,  LButtonUpEvent,   NormalState, &Machine_::Action_Drag_LButtonUp>,
			a_irow<DragState, MouseMoveEvent,                &Machine_::Action_Drag_MouseMove>,
			_row<DragState,   Exception,        NormalState>
		>
		{
		};
		using initial_state = NormalState;

		template <class FSM, class Event>
		void no_transition(Event const& e, FSM&, int state) {}

		template <class FSM, class Event>
		void exception_caught(Event const& ev, FSM& fsm, std::exception& ex)
		{
			m_pScroll->ResetStateMachine();
			throw ex;
		}

		CScrollStateMachine * const base;
		CScroll* m_pScroll;
		Machine_(CScrollStateMachine * const p, CScroll* pScroll) :base(p), m_pScroll(pScroll) {}

	};

	using Machine = boost::msm::back::state_machine<Machine_>;
	Machine m_machine;
	Impl(CScrollStateMachine * const machine, CScroll* pScroll) :m_machine(machine, pScroll) {}
};

CScrollStateMachine::CScrollStateMachine(CScroll* pScroll) :pImpl(new Impl(this, pScroll))
{
	pImpl->m_machine.start();
}
CScrollStateMachine::~CScrollStateMachine() {}

void CScrollStateMachine::LButtonDown(const LButtonDownEvent& e) { pImpl->m_machine.process_event(e); }
void CScrollStateMachine::LButtonUp(const LButtonUpEvent& e) { pImpl->m_machine.process_event(e); }
void CScrollStateMachine::MouseMove(const MouseMoveEvent& e) { pImpl->m_machine.process_event(e); }

