#include "SheetStateMachine.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "Tracker.h"
#include "Celler.h"
#include "SheetEventArgs.h"
#include "Sheet.h"
#include "Cell.h"
#include "Scroll.h"
#include "Textbox.h"
#include "TextCell.h"

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
	struct VScrlDragState :state<> 
	{
		template < class event_t, class fsm_t >
		void on_entry(event_t const & e, fsm_t & machine)
		{
			if (auto p = dynamic_cast<CGridView*>(machine.m_pSheet)) {
				p->m_pVScroll->SetState(UIElementState::Dragged);
			}
		}
		template < class event_t, class fsm_t >
		void on_exit(event_t const & e, fsm_t & machine)
		{
			if (auto p = dynamic_cast<CGridView*>(machine.m_pSheet)) {
				p->m_pVScroll->SetState(UIElementState::Normal);
			}
		}
	};
	struct HScrlDragState :state<>
	{
		template < class event_t, class fsm_t >
		void on_entry(event_t const & e, fsm_t & machine)
		{
			if (auto p = dynamic_cast<CGridView*>(machine.m_pSheet)) {
				p->m_pHScroll->SetState(UIElementState::Dragged);
			}
		}
		template < class event_t, class fsm_t >
		void on_exit(event_t const & e, fsm_t & machine)
		{
			if (auto p = dynamic_cast<CGridView*>(machine.m_pSheet)) {
				p->m_pHScroll->SetState(UIElementState::Normal);
			}
		}
	};
	struct RowDragState :state<> {};
	struct ColDragState :state<> {};
	struct ItemDragState :state<> {};
	struct EditState :state<>
	{
		template < class event_t, class fsm_t >
		void on_entry(event_t const& e, fsm_t& machine)
		{
			if (auto p = dynamic_cast<CGridView*>(machine.m_pSheet)){
				CTextCell* pCell = static_cast<CTextCell*>(e.CellPtr);

				D2DTextbox* pEdit = new D2DTextbox(
					p,
					pCell,
					pCell->GetPropertyPtr(),
					[pCell]() -> std::basic_string<TCHAR> {
						return pCell->GetString();
					},
					[pCell](const std::basic_string<TCHAR>& str) -> void {
						pCell->SetString(str);
					},
						[pCell](const std::basic_string<TCHAR>& str) -> void {
						if (pCell->CanSetStringOnEditing()) {
							pCell->SetString(str);
						}
					},
						[pCell]()->void {
						pCell->SetEditPtr(nullptr);
						pCell->GetSheetPtr()->GetGridPtr()->SetEditPtr(nullptr);
						pCell->SetState(UIElementState::Normal);//After Editing, Change Normal
					}
					);
				pEdit->OnCreate(CreateEvent(p, NULL, NULL));
				pCell->SetEditPtr(pEdit);
				p->SetEditPtr(pEdit);
			}

		}
		template < class event_t, class fsm_t >
		void on_exit(event_t const& e, fsm_t& machine)
		{
			if (auto p = dynamic_cast<CGridView*>(machine.m_pSheet)) {
				p->GetEditPtr()->OnClose(CloseEvent(p, NULL, NULL));
			}
		}
	};

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
			m_startDragV = e.WndPtr->GetDirectPtr()->Pixels2DipsY(e.Point.y);
		}

		template<class Event>
		bool Guard_VScrlDrag_LButtonDown(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				return p->m_pVScroll->GetVisible() && p->m_pVScroll->GetThumbRect().PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.Point));
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
					(e.WndPtr->GetDirectPtr()->Pixels2DipsY(e.Point.y) - m_startDragV) *
					p->m_pVScroll->GetScrollDistance() /
					p->m_pVScroll->GetRect().Height());
					m_startDragV = e.WndPtr->GetDirectPtr()->Pixels2DipsY(e.Point.y);
			}
		}

		//HScrlDrag
		FLOAT m_startDragH = 0.f;
		template<class Event>
		void Action_HScrlDrag_LButtonDown(Event const & e)
		{
			m_startDragH = e.WndPtr->GetDirectPtr()->Pixels2DipsX(e.Point.x);
		}

		template<class Event>
		bool Guard_HScrlDrag_LButtonDown(Event const & e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				return p->m_pHScroll->GetVisible() && p->m_pHScroll->GetThumbRect().PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.Point));
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
					(e.WndPtr->GetDirectPtr()->Pixels2DipsX(e.Point.x) - m_startDragH) *
					p->m_pHScroll->GetScrollDistance() /
					p->m_pHScroll->GetRect().Width());
				m_startDragH = e.WndPtr->GetDirectPtr()->Pixels2DipsX(e.Point.x);
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

		//Edit state
		template<class Event>
		void Action_Edit_MouseMove(Event const& e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				if (p->GetEditPtr()->GetClientRect().PtInRect(p->GetDirectPtr()->Pixels2Dips(e.Point))) {
					p->GetEditPtr()->OnMouseMove(e);
				}
				else {
					Action_Normal_MouseMove(e);
				}
			}
		}

		template<class Event>
		bool Guard_Edit_LButtonDown(Event const& e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				return !p->GetEditPtr()->GetClientRect().PtInRect(p->GetDirectPtr()->Pixels2Dips(e.Point));
			}
			else {
				return true;
			}
		}

		template<class Event>
		void Action_Edit_LButtonDown(Event const& e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				p->GetEditPtr()->OnLButtonDown(e);
			}
		}

		template<class Event>
		void Action_Edit_LButtonUp(Event const& e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				if (p->GetEditPtr()->GetClientRect().PtInRect(p->GetDirectPtr()->Pixels2Dips(e.Point))) {
					p->GetEditPtr()->OnLButtonUp(e);
				}
				else {
					Action_Normal_LButtonUp(e);
				}
			}
		}

		template<class Event>
		bool Guard_Edit_KeyDown(Event const& e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {

				if ((e.Char == VK_RETURN) && !(::GetKeyState(VK_MENU) & 0x8000) ||
					(e.Char == VK_TAB) && !(::GetKeyState(VK_MENU) & 0x8000)) {
					//Commit Edit
					return true;
				}
				else if (e.Char == VK_ESCAPE) {
					//CancelEdit
					p->GetEditPtr()->CancelEdit();
					return true;
				}
				else {
					return false;
				}
			}

		}

		template<class Event>
		void Action_Edit_KeyDown(const Event& e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				p->GetEditPtr()->OnKeyDown(e);
			}
		}

		template<class Event>
		void Action_Edit_Char(const Event& e)
		{
			if (auto p = dynamic_cast<CGridView*>(m_pSheet)) {
				p->GetEditPtr()->OnChar(e);
			}
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

			  _row<NormalState,   BeginEditEvent,        EditState>,
			   row<NormalState,   LButtonDownEvent,      VScrlDragState,&Machine_::Action_VScrlDrag_LButtonDown,   &Machine_::Guard_VScrlDrag_LButtonDown>,
			   row<NormalState,   LButtonDownEvent,      HScrlDragState,&Machine_::Action_HScrlDrag_LButtonDown,   &Machine_::Guard_HScrlDrag_LButtonDown>,
			   row<NormalState,   LButtonBeginDragEvent, ColDragState,  &Machine_::Action_ColDrag_LButtonBeginDrag,  &Machine_::Guard_ColDrag_LButtonBeginDrag>,
			   row<NormalState,   LButtonBeginDragEvent, RowDragState,  &Machine_::Action_RowDrag_LButtonBeginDrag,  &Machine_::Guard_RowDrag_LButtonBeginDrag>,
			   row<NormalState,   LButtonBeginDragEvent, ItemDragState, &Machine_::Action_ItemDrag_LButtonBeginDrag, &Machine_::Guard_ItemDrag_LButtonBeginDrag>,
      		   row<NormalState,   LButtonDownEvent,      RowTrackState, &Machine_::Action_RowTrack_LButtonDown, &Machine_::Guard_RowTrack_LButtonDown>,
       		   row<NormalState,   LButtonDownEvent,      ColTrackState, &Machine_::Action_ColTrack_LButtonDown, &Machine_::Guard_ColTrack_LButtonDown>,
			
			a_irow<EditState,     MouseMoveEvent,                       &Machine_::Action_Edit_MouseMove>,
			   row<EditState,	  LButtonDownEvent,      NormalState,   &Machine_::Action_Edit_LButtonDown, &Machine_::Guard_Edit_LButtonDown>,
			a_irow<EditState,     LButtonUpEvent,                       &Machine_::Action_Edit_LButtonUp>,
			   row<EditState,     KeyDownEvent,          NormalState,   &Machine_::Action_Edit_KeyDown, &Machine_::Guard_Edit_KeyDown>,
			a_irow<EditState,     CharEvent,                            &Machine_::Action_Edit_Char>,
			  _row<EditState,     KillFocusEvent,        NormalState>,



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
void CSheetStateMachine::BeginEdit(const BeginEditEvent& e) { pImpl->m_machine.process_event(e); }

