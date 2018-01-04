#include "SheetStateMachine.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "SheetEventArgs.h"
#include "Sheet.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

using namespace boost::msm::front;

struct CSheetStateMachine::Impl :state_machine_def<CSheetStateMachine::Impl>
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

	struct LButtonSnglClk :public MouseEvent
	{
		LButtonSnglClk(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct LButtonDblClk :public MouseEvent
	{
		LButtonDblClk(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct RButtonDown :public MouseEvent
	{
		RButtonDown(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};


	struct MouseMove :public MouseEvent
	{
		MouseMove(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct MouseLeave :public MouseEvent
	{
		MouseLeave(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct LButtonBeginDrag :public MouseEvent
	{
		LButtonBeginDrag(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct LButtonEndDrag :public MouseEvent
	{
		LButtonEndDrag(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	struct SetCursor
	{
		SetCursor(CSheet* pSheet, SetCursorEventArgs e) :SheetPtr(pSheet), Args(e) {}
		CSheet* SheetPtr;
		SetCursorEventArgs Args;
	};
	//State
	struct NormalState :state<> {};
	//struct RowTrackState :state<> {};
	//struct ColTrackState :state<> {};
	struct RowDragState :state<> {};
	struct ColDragState :state<> {};
	struct ItemDragState :state<> {};

	//Machine

	struct Machine_ :state_machine_def<CSheetStateMachine::Impl::Machine_>
	{
		//Normal
		template<class Event>
		void Action_Normal_LButtonDown(Event const & e)
		{
			e.SheetPtr->m_spCursorer->OnLButtonDown(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_LButtonDblClk(Event const & e)
		{
			e.SheetPtr->m_spCursorer->OnLButtonDblClk(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_RButtonDown(Event const & e)
		{
			e.SheetPtr->m_spCursorer->OnRButtonDown(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		//RowDrag
		template<class Event>
		void Action_RowDrag_LButtonBeginDrag(Event const & e)
		{
			e.SheetPtr->m_spRowDragger->OnBeginDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		bool Guard_RowDrag_LButtonBeginDrag(Event const & e)
		{
			return e.SheetPtr->m_spRowDragger->IsTarget(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_RowDrag_MouseMove(Event const & e)
		{
			e.SheetPtr->m_spRowDragger->OnDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_RowDrag_LButtonEndDrag(Event const & e)
		{
			e.SheetPtr->m_spRowDragger->OnEndDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		//ColDrag
		template<class Event>
		void Action_ColDrag_LButtonBeginDrag(Event const & e)
		{
			e.SheetPtr->m_spColDragger->OnBeginDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		bool Guard_ColDrag_LButtonBeginDrag(Event const & e)
		{
			return e.SheetPtr->m_spColDragger->IsTarget(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ColDrag_MouseMove(Event const & e)
		{
			e.SheetPtr->m_spColDragger->OnDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ColDrag_LButtonEndDrag(Event const & e)
		{
			e.SheetPtr->m_spColDragger->OnEndDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		//ItemDrag
		template<class Event>
		void Action_ItemDrag_LButtonBeginDrag(Event const & e)
		{
			e.SheetPtr->m_spItemDragger->OnBeginDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		bool Guard_ItemDrag_LButtonBeginDrag(Event const & e)
		{
			return e.SheetPtr->m_spItemDragger->IsTarget(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ItemDrag_MouseMove(Event const & e)
		{
			e.SheetPtr->m_spItemDragger->OnDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ItemDrag_LButtonEndDrag(Event const & e)
		{
			e.SheetPtr->m_spItemDragger->OnEndDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}


		struct transition_table :boost::mpl::vector<
			//     Start         Event             Target        Action                                      Guard
			a_irow<NormalState,  LButtonDown,                    &Machine_::Action_Normal_LButtonDown>,
			a_irow<NormalState,  LButtonDblClk,	                 &Machine_::Action_Normal_LButtonDblClk>,
			a_irow<NormalState,  RButtonDown,                    &Machine_::Action_Normal_RButtonDown>,
			   row<NormalState,  LButtonBeginDrag, RowDragState, &Machine_::Action_RowDrag_LButtonBeginDrag, &Machine_::Guard_RowDrag_LButtonBeginDrag>,
			   row<NormalState,  LButtonBeginDrag, ColDragState, &Machine_::Action_ColDrag_LButtonBeginDrag, &Machine_::Guard_ColDrag_LButtonBeginDrag>,
			a_irow<RowDragState, MouseMove,                      &Machine_::Action_RowDrag_MouseMove>,
			a_irow<ColDragState, MouseMove,                      &Machine_::Action_ColDrag_MouseMove>,
			 a_row<RowDragState, LButtonEndDrag,   NormalState,  &Machine_::Action_RowDrag_LButtonEndDrag>,
   			 a_row<ColDragState, LButtonEndDrag,   NormalState,  &Machine_::Action_ColDrag_LButtonEndDrag>







			> {};
		using initial_state = NormalState;

		template <class FSM, class Event>
		void no_transition(Event const& e, FSM&, int state){}

		CSheetStateMachine * const base;
		Machine_(CSheetStateMachine * const p) :base(p) {}

	};

	using Machine = boost::msm::back::state_machine<Machine_>;
	Machine machine;
	Impl(CSheetStateMachine * const p) :machine(p) {}


};

CSheetStateMachine::CSheetStateMachine() :pImpl(new Impl(this))
{
	pImpl->machine.start();
}
CSheetStateMachine::~CSheetStateMachine() {}

void CSheetStateMachine::LButtonDown(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonDown(pSheet, e)); }
void CSheetStateMachine::LButtonUp(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonUp(pSheet, e)); }
void CSheetStateMachine::LButtonSnglClk(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonSnglClk(pSheet, e)); }
void CSheetStateMachine::LButtonDblClk(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonDblClk(pSheet, e)); }
void CSheetStateMachine::RButtonDown(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::RButtonDown(pSheet, e)); }
void CSheetStateMachine::MouseMove(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::MouseMove(pSheet, e)); }
void CSheetStateMachine::MouseLeave(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::MouseLeave(pSheet, e)); }
void CSheetStateMachine::LButtonBeginDrag(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonBeginDrag(pSheet, e)); }
void CSheetStateMachine::LButtonEndDrag(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonEndDrag(pSheet, e)); }
void CSheetStateMachine::SetCursor(CSheet * pSheet, SetCursorEventArgs& e) { pImpl->machine.process_event(Impl::SetCursor(pSheet, e)); }

