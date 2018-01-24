#include "SheetStateMachine.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "Tracker.h"
#include "Celler.h"
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

	struct ContextMenu
	{
		ContextMenu(CSheet* pSheet, ContextMenuEventArgs e) :SheetPtr(pSheet), Args(e) {}
		CSheet* SheetPtr;
		ContextMenuEventArgs Args;
	};

	struct LButtonBeginDrag :public MouseEvent
	{
		LButtonBeginDrag(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	};

	//struct LButtonEndDrag :public MouseEvent
	//{
	//	LButtonEndDrag(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {}
	//};

	struct SetCursor
	{
		SetCursor(CSheet* pSheet, SetCursorEventArgs e) :SheetPtr(pSheet), Args(e) {}
		CSheet* SheetPtr;
		SetCursorEventArgs Args;
	};

	struct KeyDown
	{
		KeyDown(CSheet* pSheet, KeyEventArgs e) :SheetPtr(pSheet), Args(e) {}
		CSheet* SheetPtr;
		KeyEventArgs Args;
	};

	struct Exception{};

	//State
	struct NormalState :state<> {};
	struct RowTrackState :state<> {};
	struct ColTrackState :state<> {};
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
			e.SheetPtr->m_spCeller->OnLButtonDown(e.SheetPtr, e.Args);
			e.SheetPtr->m_spCursorer->OnLButtonDown(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_LButtonUp(Event const & e)
		{
			if (e.SheetPtr->m_spColTracker->IsTarget(e.SheetPtr, (MouseEventArgs)e.Args)) {
				e.SheetPtr->m_spColTracker->OnDividerDblClk(e.SheetPtr, (MouseEventArgs)e.Args);
			}
			else if (e.SheetPtr->m_spRowTracker->IsTarget(e.SheetPtr, (MouseEventArgs)e.Args)) {
				e.SheetPtr->m_spRowTracker->OnDividerDblClk(e.SheetPtr, (MouseEventArgs)e.Args);
			}
			else {
				e.SheetPtr->m_spCursorer->OnLButtonUp(e.SheetPtr, (MouseEventArgs)e.Args);
				e.SheetPtr->m_spCeller->OnLButtonUp(e.SheetPtr, e.Args);
			}
		}
		
		template<class Event>
		void Action_Normal_LButtonSnglClk(Event const & e)
		{
			e.SheetPtr->m_spCeller->OnLButtonSnglClk(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_LButtonDblClk(Event const & e)
		{
			e.SheetPtr->m_spCursorer->OnLButtonDblClk(e.SheetPtr, (MouseEventArgs)e.Args);
			e.SheetPtr->m_spCeller->OnLButtonDblClk(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_RButtonDown(Event const & e)
		{
			e.SheetPtr->m_spCursorer->OnRButtonDown(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_MouseMove(Event const & e)
		{
			e.SheetPtr->m_spCeller->OnMouseMove(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_MouseLeave(Event const & e)
		{
			e.SheetPtr->m_spCeller->OnMouseLeave(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_SetCursor(Event const & e)
		{
			e.SheetPtr->m_spRowTracker->OnSetCursor(e.SheetPtr, (SetCursorEventArgs)e.Args);
			e.SheetPtr->m_spColTracker->OnSetCursor(e.SheetPtr, (SetCursorEventArgs)e.Args);
			e.SheetPtr->m_spCeller->OnSetCursor(e.SheetPtr, (SetCursorEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_ContextMenu(Event const & e)
		{
			e.SheetPtr->m_spCeller->OnContextMenu(e.SheetPtr, (ContextMenuEventArgs)e.Args);
		}

		template<class Event>
		void Action_Normal_KeyDown(Event const & e)
		{
			e.SheetPtr->m_spCursorer->OnKeyDown(e.SheetPtr, (KeyEventArgs)e.Args);
			e.SheetPtr->m_spCeller->OnKeyDown(e.SheetPtr, (KeyEventArgs)e.Args);
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
		void Action_RowDrag_LButtonUp(Event const & e)
		{
			e.SheetPtr->m_spRowDragger->OnEndDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_RowDrag_MouseLeave(Event const & e)
		{
			e.SheetPtr->m_spRowDragger->OnLeaveDrag(e.SheetPtr, (MouseEventArgs)e.Args);
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
		void Action_ColDrag_LButtonUp(Event const & e)
		{
			e.SheetPtr->m_spColDragger->OnEndDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ColDrag_MouseLeave(Event const & e)
		{
			e.SheetPtr->m_spColDragger->OnLeaveDrag(e.SheetPtr, (MouseEventArgs)e.Args);
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
		void Action_ItemDrag_LButtonUp(Event const & e)
		{
			e.SheetPtr->m_spItemDragger->OnEndDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ItemDrag_MouseLeave(Event const & e)
		{
			e.SheetPtr->m_spItemDragger->OnLeaveDrag(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		//Tracker
		template<class Event>
		void Action_RowTrack_LButtonDown(Event const & e)
		{
			e.SheetPtr->m_spRowTracker->OnBeginTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		bool Guard_RowTrack_LButtonDown(Event const & e)
		{
			return e.SheetPtr->m_spRowTracker->IsTarget(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ColTrack_LButtonDown(Event const & e)
		{
			e.SheetPtr->m_spColTracker->OnBeginTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		bool Guard_ColTrack_LButtonDown(Event const & e)
		{
			return e.SheetPtr->m_spColTracker->IsTarget(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_RowTrack_MouseMove(Event const & e)
		{
			e.SheetPtr->m_spRowTracker->OnTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_RowTrack_LButtonUp(Event const & e)
		{
			e.SheetPtr->m_spRowTracker->OnEndTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_RowTrack_MouseLeave(Event const & e)
		{
			e.SheetPtr->m_spRowTracker->OnLeaveTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ColTrack_MouseMove(Event const & e)
		{
			e.SheetPtr->m_spColTracker->OnTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ColTrack_LButtonUp(Event const & e)
		{
			e.SheetPtr->m_spColTracker->OnEndTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}

		template<class Event>
		void Action_ColTrack_MouseLeave(Event const & e)
		{
			e.SheetPtr->m_spColTracker->OnLeaveTrack(e.SheetPtr, (MouseEventArgs)e.Args);
		}



		struct transition_table :boost::mpl::vector<
			//     Start          Event             Target         Action                                       Guard
			a_irow<NormalState,   LButtonDown,                     &Machine_::Action_Normal_LButtonDown>,
			a_irow<NormalState,   LButtonUp,					   &Machine_::Action_Normal_LButtonUp>,
			a_irow<NormalState,   LButtonSnglClk,                  &Machine_::Action_Normal_LButtonSnglClk>,
			a_irow<NormalState,   LButtonDblClk,	               &Machine_::Action_Normal_LButtonDblClk>,
			a_irow<NormalState,   RButtonDown,                     &Machine_::Action_Normal_RButtonDown>,
			a_irow<NormalState,   MouseMove,                       &Machine_::Action_Normal_MouseMove>,
			a_irow<NormalState,   MouseLeave,                      &Machine_::Action_Normal_MouseLeave>,
			a_irow<NormalState,   ContextMenu,                     &Machine_::Action_Normal_ContextMenu>,
			a_irow<NormalState,   SetCursor,                       &Machine_::Action_Normal_SetCursor>,
			a_irow<NormalState,   KeyDown,                         &Machine_::Action_Normal_KeyDown>,

			   row<NormalState,   LButtonBeginDrag, ColDragState,  &Machine_::Action_ColDrag_LButtonBeginDrag,  &Machine_::Guard_ColDrag_LButtonBeginDrag>,
			   row<NormalState,   LButtonBeginDrag, RowDragState,  &Machine_::Action_RowDrag_LButtonBeginDrag,  &Machine_::Guard_RowDrag_LButtonBeginDrag>,
			   row<NormalState,   LButtonBeginDrag, ItemDragState, &Machine_::Action_ItemDrag_LButtonBeginDrag, &Machine_::Guard_ItemDrag_LButtonBeginDrag>,
      		   row<NormalState,   LButtonDown, RowTrackState, &Machine_::Action_RowTrack_LButtonDown, &Machine_::Guard_RowTrack_LButtonDown>,
       		   row<NormalState,   LButtonDown, ColTrackState, &Machine_::Action_ColTrack_LButtonDown, &Machine_::Guard_ColTrack_LButtonDown>,
			
			a_irow<RowDragState,  MouseMove,                       &Machine_::Action_RowDrag_MouseMove>,
			 a_row<RowDragState,  LButtonUp,   NormalState,   &Machine_::Action_RowDrag_LButtonUp>,
			 a_row<RowDragState,  MouseLeave,       NormalState,   &Machine_::Action_RowDrag_MouseLeave>,
			  _row<RowDragState,  Exception,   NormalState>,
			
			a_irow<ColDragState,  MouseMove,                       &Machine_::Action_ColDrag_MouseMove>,
			 a_row<ColDragState, LButtonUp,   NormalState,   &Machine_::Action_ColDrag_LButtonUp>,
 			 a_row<ColDragState,  MouseLeave,       NormalState,   &Machine_::Action_ColDrag_MouseLeave>,
			_row<ColDragState, Exception, NormalState>,
			
			a_irow<ItemDragState, MouseMove,                       &Machine_::Action_ItemDrag_MouseMove>,
			 a_row<ItemDragState, LButtonUp,   NormalState,   &Machine_::Action_ItemDrag_LButtonUp>,
			 a_row<ItemDragState, MouseLeave,       NormalState,   &Machine_::Action_ItemDrag_MouseLeave>,
			_row<ItemDragState, Exception, NormalState>,


			a_irow<RowTrackState, MouseMove,                       &Machine_::Action_RowTrack_MouseMove>,
			 a_row<RowTrackState, LButtonUp,   NormalState,   &Machine_::Action_RowTrack_LButtonUp>,
			 a_row<RowTrackState, MouseLeave,       NormalState,   &Machine_::Action_RowTrack_MouseLeave>,
			_row<RowTrackState, Exception, NormalState>,

			a_irow<ColTrackState, MouseMove, &Machine_::Action_ColTrack_MouseMove>,
			a_row<ColTrackState, LButtonUp, NormalState, &Machine_::Action_ColTrack_LButtonUp>,
			a_row<ColTrackState, MouseLeave, NormalState, &Machine_::Action_ColTrack_MouseLeave>,
			_row<ColTrackState, Exception, NormalState>





			> {};
		using initial_state = NormalState;

		template <class FSM, class Event>
		void no_transition(Event const& e, FSM&, int state){}

		template <class FSM, class Event>
		void exception_caught(Event const& ev, FSM& fsm, std::exception& ex)
		{
			(ev.SheetPtr)->SetSheetStateMachine(std::make_shared<CSheetStateMachine>());
			throw ex;
		}

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
//void CSheetStateMachine::LButtonEndDrag(CSheet * pSheet, MouseEventArgs& e) { pImpl->machine.process_event(Impl::LButtonEndDrag(pSheet, e)); }
void CSheetStateMachine::SetCursor(CSheet * pSheet, SetCursorEventArgs& e) { pImpl->machine.process_event(Impl::SetCursor(pSheet, e)); }
void CSheetStateMachine::ContextMenu(CSheet * pSheet, ContextMenuEventArgs& e) { pImpl->machine.process_event(Impl::ContextMenu(pSheet, e)); }
void CSheetStateMachine::KeyDown(CSheet * pSheet, KeyEventArgs& e) { pImpl->machine.process_event(Impl::KeyDown(pSheet, e)); }

