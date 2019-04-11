#include "SheetStateMachine.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "Tracker.h"
#include "Celler.h"
#include "SheetEventArgs.h"
#include "Sheet.h"
#include "Scroll.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

using namespace boost::msm::front;

struct CSheetStateMachine::Impl :state_machine_def<CSheetStateMachine::Impl>
{
	//Event
	struct Exception{};

	//State
	struct NormalState :state<> {};
	struct RowTrackState :state<> {};
	struct ColTrackState :state<> {};
	struct VScrlDragState :state<> {};
	struct HScrlDragState :state<> {};
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
			m_pSheet->m_spCeller->OnLButtonDown(m_pSheet, e);
			m_pSheet->m_spCursorer->OnLButtonDown(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_LButtonUp(Event const & e)
		{
			m_pSheet->m_spCursorer->OnLButtonUp(m_pSheet, e);
			m_pSheet->m_spCeller->OnLButtonUp(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_LButtonClk(Event const & e)
		{
			m_pSheet->m_spCeller->OnLButtonClk(m_pSheet, e);
		}
		
		template<class Event>
		void Action_Normal_LButtonSnglClk(Event const & e)
		{
			m_pSheet->m_spCeller->OnLButtonSnglClk(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_LButtonDblClk(Event const & e)
		{
			if (m_pSheet->m_spColTracker->IsTarget(m_pSheet, e)) {
				m_pSheet->m_spColTracker->OnDividerDblClk(m_pSheet, e);
			}
			else if (m_pSheet->m_spRowTracker->IsTarget(m_pSheet, e)) {
				m_pSheet->m_spRowTracker->OnDividerDblClk(m_pSheet, e);
			}
			else {
				m_pSheet->m_spCursorer->OnLButtonDblClk(m_pSheet, e);
				m_pSheet->m_spCeller->OnLButtonDblClk(m_pSheet, e);
			}
		}

		template<class Event>
		void Action_Normal_RButtonDown(Event const & e)
		{
			m_pSheet->m_spCursorer->OnRButtonDown(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_MouseMove(Event const & e)
		{
			m_pSheet->m_spCeller->OnMouseMove(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_MouseLeave(Event const & e)
		{
			m_pSheet->m_spCeller->OnMouseLeave(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_SetCursor(Event const & e)
		{
			m_pSheet->m_spRowTracker->OnSetCursor(m_pSheet, e);
			m_pSheet->m_spColTracker->OnSetCursor(m_pSheet, e);
			m_pSheet->m_spCeller->OnSetCursor(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_ContextMenu(Event const & e)
		{
			m_pSheet->m_spCeller->OnContextMenu(m_pSheet, e);
		}

		template<class Event>
		void Action_Normal_KeyDown(Event const & e)
		{
			m_pSheet->m_spCursorer->OnKeyDown(m_pSheet, e);
			m_pSheet->m_spCeller->OnKeyDown(m_pSheet, e);
		}
		
		//VScrlDrag
		FLOAT m_startDragV = 0.f;
		template<class Event>
		void Action_VScrlDrag_LButtonDown(Event const & e)
		{
			m_startDragV = e.Direct.Pixels2DipsY(e.Point.y);
		}

		template<class Event>
		bool Guard_VScrlDrag_LButtonDown(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				return p->m_pVScroll->GetThumbRect().PtInRect(e.Direct.Pixels2Dips(e.Point));
			} else {
				return false;
			}
		}

		template<class Event>
		void Action_VScrlDrag_LButtonUp(Event const & e)
		{
			m_startDragV = 0.f;
		}

		template<class Event>
		void Action_VScrlDrag_MouseMove(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)){
				p->m_pVScroll->SetScrollPos(
					p->m_pVScroll->GetScrollPos() +
					(e.Direct.Pixels2DipsY(e.Point.y) - m_startDragV) *
					p->m_pVScroll->GetScrollDistance() /
					p->m_pVScroll->GetRect().Height());
					m_startDragV = e.Direct.Pixels2DipsY(e.Point.y);
			}
		}

		//HScrlDrag
		FLOAT m_startDragH = 0.f;
		template<class Event>
		void Action_HScrlDrag_LButtonDown(Event const & e)
		{
			m_startDragH = e.Direct.Pixels2DipsX(e.Point.x);
		}

		template<class Event>
		bool Guard_HScrlDrag_LButtonDown(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				return p->m_pHScroll->GetThumbRect().PtInRect(e.Direct.Pixels2Dips(e.Point));
			} else {
				return false;
			}
		}

		template<class Event>
		void Action_HScrlDrag_LButtonUp(Event const & e)
		{
			m_startDragH = 0.f;
		}

		template<class Event>
		void Action_HScrlDrag_MouseMove(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				p->m_pHScroll->SetScrollPos(
					p->m_pHScroll->GetScrollPos() +
					(e.Direct.Pixels2DipsX(e.Point.x) - m_startDragH) *
					p->m_pHScroll->GetScrollDistance() /
					p->m_pHScroll->GetRect().Width());
				m_startDragH = e.Direct.Pixels2DipsX(e.Point.x);
			}
		}



		//RowDrag
		template<class Event>
		void Action_RowDrag_LButtonBeginDrag(Event const & e)
		{
			m_pSheet->m_spRowDragger->OnBeginDrag(m_pSheet, e);
		}

		template<class Event>
		bool Guard_RowDrag_LButtonBeginDrag(Event const & e)
		{
			return m_pSheet->m_spRowDragger->IsTarget(m_pSheet, e);
		}

		template<class Event>
		void Action_RowDrag_MouseMove(Event const & e)
		{
			m_pSheet->m_spRowDragger->OnDrag(m_pSheet, e);
		}

		template<class Event>
		void Action_RowDrag_LButtonUp(Event const & e)
		{
			m_pSheet->m_spRowDragger->OnEndDrag(m_pSheet, e);
		}

		template<class Event>
		void Action_RowDrag_MouseLeave(Event const & e)
		{
			m_pSheet->m_spRowDragger->OnLeaveDrag(m_pSheet, e);
		}

		//ColDrag
		template<class Event>
		void Action_ColDrag_LButtonBeginDrag(Event const & e)
		{
			m_pSheet->m_spColDragger->OnBeginDrag(m_pSheet, e);
		}

		template<class Event>
		bool Guard_ColDrag_LButtonBeginDrag(Event const & e)
		{
			return m_pSheet->m_spColDragger->IsTarget(m_pSheet, e);
		}

		template<class Event>
		void Action_ColDrag_MouseMove(Event const & e)
		{
			m_pSheet->m_spColDragger->OnDrag(m_pSheet, e);
		}

		template<class Event>
		void Action_ColDrag_LButtonUp(Event const & e)
		{
			m_pSheet->m_spColDragger->OnEndDrag(m_pSheet, e);
		}

		template<class Event>
		void Action_ColDrag_MouseLeave(Event const & e)
		{
			m_pSheet->m_spColDragger->OnLeaveDrag(m_pSheet, e);
		}

		//ItemDrag
		template<class Event>
		void Action_ItemDrag_LButtonBeginDrag(Event const & e)
		{
			m_pSheet->m_spItemDragger->OnBeginDrag(m_pSheet, e);
		}

		template<class Event>
		bool Guard_ItemDrag_LButtonBeginDrag(Event const & e)
		{
			return m_pSheet->m_spItemDragger->IsTarget(m_pSheet, e);
		}

		template<class Event>
		void Action_ItemDrag_MouseMove(Event const & e)
		{
			m_pSheet->m_spItemDragger->OnDrag(m_pSheet, e);
		}

		template<class Event>
		void Action_ItemDrag_LButtonUp(Event const & e)
		{
			m_pSheet->m_spCursorer->OnLButtonUp(m_pSheet, e);
			m_pSheet->m_spCeller->OnLButtonUp(m_pSheet, e);

			m_pSheet->m_spItemDragger->OnEndDrag(m_pSheet, e);
		}

		template<class Event>
		void Action_ItemDrag_MouseLeave(Event const & e)
		{
			m_pSheet->m_spItemDragger->OnLeaveDrag(m_pSheet, e);
		}

		//Tracker
		template<class Event>
		void Action_RowTrack_LButtonDown(Event const & e)
		{
			m_pSheet->m_spRowTracker->OnBeginTrack(m_pSheet, e);
		}

		template<class Event>
		bool Guard_RowTrack_LButtonDown(Event const & e)
		{
			return m_pSheet->m_spRowTracker->IsTarget(m_pSheet, e);
		}

		template<class Event>
		void Action_ColTrack_LButtonDown(Event const & e)
		{
			m_pSheet->m_spColTracker->OnBeginTrack(m_pSheet, e);
		}

		template<class Event>
		bool Guard_ColTrack_LButtonDown(Event const & e)
		{
			return m_pSheet->m_spColTracker->IsTarget(m_pSheet, e);
		}

		template<class Event>
		void Action_RowTrack_MouseMove(Event const & e)
		{
			m_pSheet->m_spRowTracker->OnTrack(m_pSheet, e);
		}

		template<class Event>
		void Action_RowTrack_LButtonUp(Event const & e)
		{
			m_pSheet->m_spRowTracker->OnEndTrack(m_pSheet, e);
		}

		template<class Event>
		void Action_RowTrack_MouseLeave(Event const & e)
		{
			m_pSheet->m_spRowTracker->OnLeaveTrack(m_pSheet, e);
		}

		template<class Event>
		void Action_ColTrack_MouseMove(Event const & e)
		{
			m_pSheet->m_spColTracker->OnTrack(m_pSheet, e);
		}

		template<class Event>
		void Action_ColTrack_LButtonUp(Event const & e)
		{
			m_pSheet->m_spColTracker->OnEndTrack(m_pSheet, e);
		}

		template<class Event>
		void Action_ColTrack_MouseLeave(Event const & e)
		{
			m_pSheet->m_spColTracker->OnLeaveTrack(m_pSheet, e);
		}



		struct transition_table :boost::mpl::vector<
			//     Start          Event             Target         Action                                       Guard
			a_irow<NormalState,   LButtonDownEvent,                     &Machine_::Action_Normal_LButtonDown>,
			a_irow<NormalState,   LButtonUpEvent,					   &Machine_::Action_Normal_LButtonUp>,
			a_irow<NormalState,   LButtonClkEvent,                     &Machine_::Action_Normal_LButtonClk>,
			a_irow<NormalState,   LButtonSnglClkEvent,                  &Machine_::Action_Normal_LButtonSnglClk>,
			a_irow<NormalState,   LButtonDblClkEvent,	               &Machine_::Action_Normal_LButtonDblClk>,
			a_irow<NormalState,   RButtonDownEvent,                     &Machine_::Action_Normal_RButtonDown>,
			a_irow<NormalState,   MouseMoveEvent,                       &Machine_::Action_Normal_MouseMove>,
			a_irow<NormalState,   MouseLeaveEvent,                      &Machine_::Action_Normal_MouseLeave>,
			a_irow<NormalState,   ContextMenuEvent,                     &Machine_::Action_Normal_ContextMenu>,
			a_irow<NormalState,   SetCursorEvent,                       &Machine_::Action_Normal_SetCursor>,
			a_irow<NormalState,   KeyDownEvent,                         &Machine_::Action_Normal_KeyDown>,

			   row<NormalState,   LButtonDownEvent,      VScrlDragState,&Machine_::Action_VScrlDrag_LButtonDown,   &Machine_::Guard_VScrlDrag_LButtonDown>,
			   row<NormalState,   LButtonDownEvent,      HScrlDragState,&Machine_::Action_HScrlDrag_LButtonDown,   &Machine_::Guard_HScrlDrag_LButtonDown>,
			   row<NormalState,   LButtonBeginDragEvent, ColDragState,  &Machine_::Action_ColDrag_LButtonBeginDrag,  &Machine_::Guard_ColDrag_LButtonBeginDrag>,
			   row<NormalState,   LButtonBeginDragEvent, RowDragState,  &Machine_::Action_RowDrag_LButtonBeginDrag,  &Machine_::Guard_RowDrag_LButtonBeginDrag>,
			   row<NormalState,   LButtonBeginDragEvent, ItemDragState, &Machine_::Action_ItemDrag_LButtonBeginDrag, &Machine_::Guard_ItemDrag_LButtonBeginDrag>,
      		   row<NormalState,   LButtonDownEvent, RowTrackState, &Machine_::Action_RowTrack_LButtonDown, &Machine_::Guard_RowTrack_LButtonDown>,
       		   row<NormalState,   LButtonDownEvent, ColTrackState, &Machine_::Action_ColTrack_LButtonDown, &Machine_::Guard_ColTrack_LButtonDown>,
			
			a_irow<VScrlDragState, MouseMoveEvent, &Machine_::Action_VScrlDrag_MouseMove>,
			a_row<VScrlDragState,  LButtonUpEvent, NormalState, &Machine_::Action_VScrlDrag_LButtonUp>,

			a_irow<HScrlDragState, MouseMoveEvent, &Machine_::Action_HScrlDrag_MouseMove>,
			a_row<HScrlDragState, LButtonUpEvent, NormalState, &Machine_::Action_HScrlDrag_LButtonUp>,

			a_irow<RowDragState,  MouseMoveEvent,                       &Machine_::Action_RowDrag_MouseMove>,
			 a_row<RowDragState,  LButtonUpEvent,   NormalState,   &Machine_::Action_RowDrag_LButtonUp>,
			 a_row<RowDragState,  MouseLeaveEvent,       NormalState,   &Machine_::Action_RowDrag_MouseLeave>,
			  _row<RowDragState,  Exception,   NormalState>,
			
			a_irow<ColDragState,  MouseMoveEvent,                       &Machine_::Action_ColDrag_MouseMove>,
			 a_row<ColDragState,  LButtonUpEvent,   NormalState,   &Machine_::Action_ColDrag_LButtonUp>,
 			 a_row<ColDragState,  MouseLeaveEvent,       NormalState,   &Machine_::Action_ColDrag_MouseLeave>,
			_row<ColDragState, Exception, NormalState>,
			
			a_irow<ItemDragState, MouseMoveEvent,                       &Machine_::Action_ItemDrag_MouseMove>,
			 a_row<ItemDragState, LButtonUpEvent,   NormalState,   &Machine_::Action_ItemDrag_LButtonUp>,
			 a_row<ItemDragState, MouseLeaveEvent,       NormalState,   &Machine_::Action_ItemDrag_MouseLeave>,
			_row<ItemDragState, Exception, NormalState>,


			a_irow<RowTrackState, MouseMoveEvent,                       &Machine_::Action_RowTrack_MouseMove>,
			 a_row<RowTrackState, LButtonUpEvent,   NormalState,   &Machine_::Action_RowTrack_LButtonUp>,
			 a_row<RowTrackState, MouseLeaveEvent,       NormalState,   &Machine_::Action_RowTrack_MouseLeave>,
			_row<RowTrackState, Exception, NormalState>,

			a_irow<ColTrackState, MouseMoveEvent, &Machine_::Action_ColTrack_MouseMove>,
			a_row<ColTrackState, LButtonUpEvent, NormalState, &Machine_::Action_ColTrack_LButtonUp>,
			a_row<ColTrackState, MouseLeaveEvent, NormalState, &Machine_::Action_ColTrack_MouseLeave>,
			_row<ColTrackState, Exception, NormalState>
			> {};
		using initial_state = NormalState;

		template <class FSM, class Event>
		void no_transition(Event const& e, FSM&, int state){}

		template <class FSM, class Event>
		void exception_caught(Event const& ev, FSM& fsm, std::exception& ex)
		{
			m_pSheet->SetSheetStateMachine(std::make_shared<CSheetStateMachine>(m_pSheet));
			throw ex;
		}

		CSheetStateMachine * const base;
		CSheet* m_pSheet;
		Machine_(CSheetStateMachine * const p, CSheet* pSheet) :base(p), m_pSheet(pSheet) {}

	};

	using Machine = boost::msm::back::state_machine<Machine_>;
	Machine m_machine;
	Impl(CSheetStateMachine * const machine, CSheet* pSheet) :m_machine(machine, pSheet) {}
};

CSheetStateMachine::CSheetStateMachine(CSheet* pSheet) :pImpl(new Impl(this, pSheet))
{
	pImpl->m_machine.start();
}
CSheetStateMachine::~CSheetStateMachine() {}

void CSheetStateMachine::LButtonDown(const LButtonDownEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::LButtonUp(const LButtonUpEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::LButtonClk(const LButtonClkEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::LButtonSnglClk(const LButtonSnglClkEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::LButtonDblClk(const LButtonDblClkEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::RButtonDown(const RButtonDownEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::MouseMove(const MouseMoveEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::MouseLeave(const MouseLeaveEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::LButtonBeginDrag(const LButtonBeginDragEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::SetCursor(const SetCursorEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::ContextMenu(const ContextMenuEvent& e) { pImpl->m_machine.process_event(e); }
void CSheetStateMachine::KeyDown(const KeyDownEvent& e) { pImpl->m_machine.process_event(e); }

