#include "MouseStateMachine.h"
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
	struct MouseEvent
	{
		MouseEvent(CSheet* pSheet, MouseEventArgs e) :SheetPtr(pSheet), Args(e) {}
		CSheet* SheetPtr;
		MouseEventArgs Args;
	};

	struct LButtonDown :public MouseEvent
	{
		LButtonDown(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct LButtonUp :public MouseEvent
	{
		LButtonUp(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct LButtonDblClk :public MouseEvent
	{
		LButtonDblClk(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct LButtonDblClkTimeExceed :public MouseEvent
	{
		LButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct MouseLeave :public MouseEvent
	{
		MouseLeave(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct Exception{};


	//State
	struct NormalState :state<> { };
	struct LButtonDownedState :state<> 
	{
		template < class event_t, class fsm_t >
		void on_entry(event_t const & e, fsm_t & machine)
		{
			if (auto p = dynamic_cast<CGridView*>(e.SheetPtr)) {
				boost::asio::deadline_timer* pTimer = p->GetTimerPtr();
				pTimer->expires_from_now(boost::posix_time::milliseconds(::GetDoubleClickTime()));
				HWND hWnd = e.SheetPtr->GetGridPtr()->m_hWnd;
				pTimer->async_wait([hWnd, e](const boost::system::error_code& error)->void {

					if (error == boost::asio::error::operation_aborted) {
						std::cout << "timer canceled" << std::endl;
					}
					else {
						std::cout << "timer editcell" << std::endl;
						::PostMessage(hWnd, WM_LBUTTONDBLCLKTIMEXCEED, NULL, MAKELPARAM(e.Args.Point.x, e.Args.Point.y));
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
			e.SheetPtr->OnLButtonBeginDrag((MouseEventArgs)e.Args);
		}
	};

	//Machine
	struct Machine_ :state_machine_def<CMouseStateMachine::Impl::Machine_>
	{
		//Any
		template<class Event>
		void Action_Any_MouseLeave(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(e.SheetPtr)) {
				p->GetTimerPtr()->cancel();
			}
			e.SheetPtr->OnMouseLeave((MouseEventArgs)e.Args);
		}
		
		//Normal
		template<class Event>
		void Action_Normal_LButtonDown(Event const & e)
		{
			e.SheetPtr->OnLButtonDown((MouseEventArgs)e.Args);
		}

		//Upped
		template<class Event>
		void Action_Upped_LButtonDblClk(Event const & e)
		{
			e.SheetPtr->OnLButtonDblClk((MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Upped_LButtonDblClkTimeExceed(Event const & e)
		{
			e.SheetPtr->OnLButtonSnglClk((MouseEventArgs)e.Args);
			if (auto p = dynamic_cast<CGridView*>(e.SheetPtr)) {
				p->GetTimerPtr()->cancel();
			}
		}

		//Drag
		template<class Event>
		void Action_LButtonUp(Event const & e)
		{	
			e.SheetPtr->OnLButtonUp((MouseEventArgs)e.Args);
		}

		struct transition_table :boost::mpl::vector<
			//     Start                 Event                    Target                Action                                             Guard
			 a_row<NormalState,          LButtonDown,             LButtonDownedState,   &Machine_::Action_Normal_LButtonDown>,
			 a_row<NormalState,          MouseLeave,              NormalState,          &Machine_::Action_Any_MouseLeave>,

			  _row<LButtonDownedState,   LButtonDblClkTimeExceed, LButtonDragState>,
 			 a_row<LButtonDownedState,   LButtonUp,               LButtonUppedState,    &Machine_::Action_LButtonUp>,
     		 a_row<LButtonDownedState,   MouseLeave,              NormalState,          &Machine_::Action_Any_MouseLeave>,
			 _row<LButtonDownedState,    Exception,               NormalState>,

			 a_row<LButtonUppedState,    LButtonDblClk,           LButtonDblClkedState, &Machine_::Action_Upped_LButtonDblClk>,
 			 a_row<LButtonUppedState,    LButtonDblClkTimeExceed, NormalState,          &Machine_::Action_Upped_LButtonDblClkTimeExceed>,
			 a_row<LButtonUppedState,    MouseLeave,              NormalState,          &Machine_::Action_Any_MouseLeave>,
			_row<LButtonUppedState,      Exception,               NormalState>,

			 a_row<LButtonDblClkedState, LButtonUp,               NormalState,          &Machine_::Action_LButtonUp>,
			 a_row<LButtonDblClkedState, MouseLeave,              NormalState,          &Machine_::Action_Any_MouseLeave>,
			_row<LButtonDblClkedState,   Exception,               NormalState>,

			 a_row<LButtonDragState,     LButtonUp,               NormalState,          &Machine_::Action_LButtonUp>,
			 a_row<LButtonDragState,     MouseLeave,              NormalState,          &Machine_::Action_Any_MouseLeave>,
			_row<LButtonDragState,       Exception,               NormalState>


		> {};

		using initial_state = NormalState;

		template <class FSM, class Event>
		void no_transition(Event const& e, FSM&, int state) {}

		template <class FSM, class Event>
		void exception_caught(Event const&, FSM& fsm, std::exception& e)
		{
			fsm.process_event(Impl::Exception());
			throw e;
		}

		CMouseStateMachine * const base;
		Machine_(CMouseStateMachine * const p) :base(p) {}

	};

	using Machine = boost::msm::back::state_machine<Machine_>;
	Machine machine;
	Impl(CMouseStateMachine * const p) :machine(p) {}


};

CMouseStateMachine::CMouseStateMachine() :pImpl(new Impl(this))
{
	pImpl->machine.start();
}
CMouseStateMachine::~CMouseStateMachine() {}

void CMouseStateMachine::LButtonDown(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonDown(pSheet, e)); }
void CMouseStateMachine::LButtonUp(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonUp(pSheet, e)); }
void CMouseStateMachine::LButtonDblClk(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonDblClk(pSheet, e)); }
void CMouseStateMachine::LButtonDblClkTimeExceed(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonDblClkTimeExceed(pSheet, e)); }
void CMouseStateMachine::MouseLeave(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::MouseLeave(pSheet, e)); }
