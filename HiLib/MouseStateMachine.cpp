#include "MouseStateMachine.h"
#include "UIElement.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

using namespace boost::msm::front;

struct CMouseStateMachine::Impl :state_machine_def<CMouseStateMachine::Impl>
{
	//Event
	struct Exception{};

	//State
	struct NormalState :state<> 
	{
		template < class event_t, class fsm_t >
		void on_entry(event_t const & e, fsm_t & machine)
		{
			std::cout << "NormalState" << std::endl;
		}
	};
	struct LButtonDownedState :state<> 
	{
		template < class event_t, class fsm_t >
		void on_entry(event_t const & e, fsm_t & machine)
		{
			std::cout << "LButtonDownedState" << std::endl;
			if (auto p = dynamic_cast<CGridView*>(machine.m_pSheet)) {
				boost::asio::deadline_timer* pTimer = p->GetFilterTimerPtr();
				pTimer->expires_from_now(boost::posix_time::milliseconds(::GetDoubleClickTime()));
				HWND hWnd = p->m_hWnd;
				pTimer->async_wait([hWnd, e](const boost::system::error_code& error)->void {

					if (error == boost::asio::error::operation_aborted) {
						std::cout << "timer canceled" << std::endl;
					}
					else {
						std::cout << "timer editcell" << std::endl;
						::PostMessage(hWnd, WM_LBUTTONDBLCLKTIMEXCEED, NULL, MAKELPARAM(e.Point.x, e.Point.y));
					}
				});
			}
		}
	};
	struct LButtonUppedState :state<> {};
	struct LButtonDblClkedState :state<> {};
	struct LButtonDragState :state<> 
	{
		template < class event_t, class fsm_t >
		void on_entry(event_t const & e, fsm_t & machine)
		{
			std::cout << "LButtonDragState" << std::endl;
			machine.m_pSheet->OnLButtonBeginDrag(LButtonBeginDragEvent(e.Flags, e.Point));
		}
	};

	//Machine
	struct Machine_ :state_machine_def<CMouseStateMachine::Impl::Machine_>
	{
		//Any
		template<class Event>
		void Action_LButtonUp(Event const & e)
		{
			m_pSheet->OnLButtonClk(LButtonClkEvent(e.Flags, e.Point));
			m_pSheet->OnLButtonUp(e);
		}

		template<class Event>
		void Action_MouseLeave(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				p->GetFilterTimerPtr()->cancel();
			}
			m_pSheet->OnMouseLeave(e);
		}
		
		template<class Event>
		void Action_LButtonDown(Event const & e)
		{
			m_pSheet->OnLButtonDown(e);
		}

		//Upped
		template<class Event>
		void Action_Upped_LButtonDblClk(Event const & e)
		{
			m_pSheet->OnLButtonDblClk(e);
		}

		template<class Event>
		void Action_Upped_LButtonDblClkTimeExceed(Event const & e)
		{
			m_pSheet->OnLButtonSnglClk(LButtonSnglClkEvent(e.Flags, e.Point));
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				p->GetFilterTimerPtr()->cancel();
			}
		}

		//Drag
		template<class Event>
		void Action_Drag_MouseLeave(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				p->GetFilterTimerPtr()->cancel();
			}
			//m_pSheet->OnLButtonUp((MouseEventArgs)e.Args);
			m_pSheet->OnMouseLeave(e);
		}

		struct transition_table :boost::mpl::vector<
			//     Start                 Event                    Target                Action                                             Guard
			 a_row<NormalState,          LButtonDownEvent,             LButtonDownedState,   &Machine_::Action_LButtonDown>,
			 a_row<NormalState,          MouseLeaveEvent,              NormalState,          &Machine_::Action_MouseLeave>,

			  _row<LButtonDownedState,   LButtonDblClkTimeExceedEvent, LButtonDragState>,
 			 a_row<LButtonDownedState,   LButtonUpEvent,               LButtonUppedState,    &Machine_::Action_LButtonUp>,
     		 a_row<LButtonDownedState,   MouseLeaveEvent,              NormalState,          &Machine_::Action_MouseLeave>,
			 _row<LButtonDownedState,    Exception,               NormalState>,

			 a_row<LButtonUppedState,    LButtonDownEvent,              LButtonDownedState, &Machine_::Action_LButtonDown>,
			a_row<LButtonUppedState,     LButtonDblClkEvent,           LButtonDblClkedState, &Machine_::Action_Upped_LButtonDblClk>,
 			 a_row<LButtonUppedState,    LButtonDblClkTimeExceedEvent, NormalState,          &Machine_::Action_Upped_LButtonDblClkTimeExceed>,
			 a_row<LButtonUppedState,    MouseLeaveEvent,              NormalState,          &Machine_::Action_MouseLeave>,
			_row<LButtonUppedState,      Exception,               NormalState>,

			 a_row<LButtonDblClkedState, LButtonUpEvent,               NormalState,          &Machine_::Action_LButtonUp>,
			 a_row<LButtonDblClkedState, MouseLeaveEvent,              NormalState,          &Machine_::Action_MouseLeave>,
			_row<LButtonDblClkedState,   Exception,               NormalState>,

			 a_row<LButtonDragState,     LButtonUpEvent,               NormalState,          &Machine_::Action_LButtonUp>,
			 a_row<LButtonDragState,     MouseLeaveEvent,              NormalState,          &Machine_::Action_Drag_MouseLeave>,
			_row<LButtonDragState,       Exception,               NormalState>


		> {};

		using initial_state = NormalState;

		template <class FSM, class Event>
		void no_transition(Event const& e, FSM&, int state) {}

		template <class FSM, class Event>
		void exception_caught(Event const& ev, FSM& fsm, std::exception& ex)
		{
			reinterpret_cast<CGridView*>(m_pSheet)->SetMouseStateMachine(std::make_shared<CMouseStateMachine>(m_pSheet));
			throw ex;
		}

		CMouseStateMachine * const base;
		CSheet* m_pSheet;
		Machine_(CMouseStateMachine * const p, CSheet* pSheet) :base(p), m_pSheet(pSheet) {}

	};

	using Machine = boost::msm::back::state_machine<Machine_>;
	Machine m_machine;
	Impl(CMouseStateMachine * const p, CSheet* pSheet) :m_machine(p, pSheet) {}


};

CMouseStateMachine::CMouseStateMachine(CSheet* pSheet) :pImpl(new Impl(this, pSheet))
{
	pImpl->m_machine.start();
}
CMouseStateMachine::~CMouseStateMachine() {}

void CMouseStateMachine::LButtonDown(const LButtonDownEvent& e) { pImpl->m_machine.process_event(e); }
void CMouseStateMachine::LButtonUp(const LButtonUpEvent& e) { pImpl->m_machine.process_event(e); }
void CMouseStateMachine::LButtonDblClk(const LButtonDblClkEvent& e) { pImpl->m_machine.process_event(e); }
void CMouseStateMachine::LButtonDblClkTimeExceed(const LButtonDblClkTimeExceedEvent& e) { pImpl->m_machine.process_event(e); }
void CMouseStateMachine::MouseLeave(const MouseLeaveEvent& e) { pImpl->m_machine.process_event(e); }
