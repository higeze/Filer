//#pragma once
//#include "GridViewStateMachine.h"
//#include "GridView.h"
//#include "Celler.h"
//#include "Cursorer.h"
//#include "Tracker.h"
//#include "Dragger.h"
//#include "Scroll.h"
//#include "Textbox.h"
//#include <boost/sml.hpp>
//
//namespace sml = boost::sml;
//
//struct CGridViewStateMachine::Impl
//{
//
//	class Normal
//	{
//	public:
//		static void LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCeller->OnLButtonDown(pGrid, e);
//			pGrid->m_spCursorer->OnLButtonDown(pGrid, e);
//		}
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCursorer->OnLButtonUp(pGrid, e);
//			pGrid->m_spCeller->OnLButtonUp(pGrid, e);
//		}
//		static void LButtonClk(CGridView* p, const LButtonClkEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCeller->OnLButtonClk(pGrid, e);
//		}
//		static void LButtonSnglClk(CGridView* p, const LButtonSnglClkEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCeller->OnLButtonSnglClk(pGrid, e);
//		}
//		static void LButtonDblClk(CGridView* p, const LButtonDblClkEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			if (pGrid->m_spColTracker->IsTarget(pGrid, e)) {
//				pGrid->m_spColTracker->OnDividerDblClk(pGrid, e);
//			}
//			else if (pGrid->m_spRowTracker->IsTarget(pGrid, e)) {
//				pGrid->m_spRowTracker->OnDividerDblClk(pGrid, e);
//			}
//			else {
//				pGrid->m_spCursorer->OnLButtonDblClk(pGrid, e);
//				pGrid->m_spCeller->OnLButtonDblClk(pGrid, e);
//			}
//		}
//		static void RButtonDown(CGridView* p, const RButtonDownEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCursorer->OnRButtonDown(pGrid, e);
//		}
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCeller->OnMouseMove(pGrid, e);
//		}
//		static void MouseLeave(CGridView* p, const MouseLeaveEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCeller->OnMouseLeave(pGrid, e);
//		}
//		static void SetCursor(CGridView* p, SetCursorEvent const& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spRowTracker->OnSetCursor(pGrid, e);
//			pGrid->m_spColTracker->OnSetCursor(pGrid, e);
//			pGrid->m_spCeller->OnSetCursor(pGrid, e);
//		}
//		static void ContextMenu(CGridView* p, const ContextMenuEvent & e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCeller->OnContextMenu(pGrid, e);
//		}
//		static void KeyDown(CGridView* p, const KeyDownEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_spCursorer->OnKeyDown(pGrid, e);
//			pGrid->m_spCeller->OnKeyDown(pGrid, e);
//		}
//	};
//	class RowTrack
//	{
//	public:
//		static void LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			p->m_spRowTracker->OnBeginTrack(p, e);
//		}
//
//		static bool Guard_LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			return p->m_spRowTracker->IsTarget(p, e);
//		}
//
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			p->m_spRowTracker->OnTrack(p, e);
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			p->m_spRowTracker->OnEndTrack(p, e);
//		}
//
//		static void MouseLeave(CGridView* p, const MouseLeaveEvent& e)
//		{
//			p->m_spRowTracker->OnLeaveTrack(p, e);
//		}
//	};
//	class ColTrack
//	{
//	public:
//		static void LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			p->m_spColTracker->OnBeginTrack(p, e);
//		}
//
//		static bool Guard_LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			return p->m_spColTracker->IsTarget(p, e);
//		}
//
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			p->m_spColTracker->OnTrack(p, e);
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			p->m_spColTracker->OnEndTrack(p, e);
//		}
//
//		static void MouseLeave(CGridView* p, const MouseLeaveEvent& e)
//		{
//			p->m_spColTracker->OnLeaveTrack(p, e);
//		}
//	};
//
//	class VsclDrag
//	{
//	public:
//		static void OnEntry(CGridView* p)
//		{
//			p->m_pVScroll->SetState(UIElementState::Dragged);
//		}
//
//		static void OnExit(CGridView* p)
//		{
//			p->m_pVScroll->SetState(UIElementState::Normal);
//		}
//
//		static void LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_pVScroll->SetStartDrag(e.WndPtr->GetDirectPtr()->Pixels2DipsY(e.Point.y));
//		}
//
//		static bool Guard_LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			return pGrid->m_pVScroll->GetVisible() && pGrid->m_pVScroll->GetThumbRect().PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.Point));
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_pVScroll->SetStartDrag(0.f);
//		}
//
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_pVScroll->SetScrollPos(
//			pGrid->m_pVScroll->GetScrollPos() +
//			(pGrid->GetDirectPtr()->Pixels2DipsY(e.Point.y) - pGrid->m_pVScroll->GetStartDrag()) *
//			pGrid->m_pVScroll->GetScrollDistance() /
//			pGrid->m_pVScroll->GetRect().Height());
//			pGrid->m_pVScroll->SetStartDrag(e.WndPtr->GetDirectPtr()->Pixels2DipsY(e.Point.y));
//		}
//
//	};
//	class HsclDrag
//	{
//	public:
//		static void OnEntry(CGridView* p)
//		{
//			p->m_pHScroll->SetState(UIElementState::Dragged);
//		}
//
//		static void OnExit(CGridView* p)
//		{
//			p->m_pHScroll->SetState(UIElementState::Normal);
//		}
//
//		static void LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			p->m_pHScroll->SetStartDrag(p->GetDirectPtr()->Pixels2DipsX(e.Point.x));
//		}
//
//		static bool Guard_LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			return p->m_pHScroll->GetVisible() && p->m_pVScroll->GetThumbRect().PtInRect(p->GetDirectPtr()->Pixels2Dips(e.Point));
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			p->m_pHScroll->SetStartDrag(0.f);
//		}
//
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			auto pGrid = static_cast<CGridView*>(e.WndPtr);
//			pGrid->m_pHScroll->SetScrollPos(
//				pGrid->m_pHScroll->GetScrollPos() +
//				(pGrid->GetDirectPtr()->Pixels2DipsX(e.Point.x) - pGrid->m_pHScroll->GetStartDrag()) *
//				pGrid->m_pHScroll->GetScrollDistance() /
//				pGrid->m_pHScroll->GetRect().Height());
//			pGrid->m_pHScroll->SetStartDrag(e.WndPtr->GetDirectPtr()->Pixels2DipsX(e.Point.x));
//		}
//
//	};
//
//	class RowDrag
//	{
//	public:
//		static void LButtonBeginDrag(CGridView* p, const LButtonBeginDragEvent& e)
//		{
//			p->m_spRowDragger->OnBeginDrag(p, e);
//		}
//
//		static bool Guard_LButtonBeginDrag(CGridView* p, const LButtonBeginDragEvent& e)
//		{
//			return p->m_spRowDragger->IsTarget(p, e);
//		}
//
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			p->m_spRowDragger->OnDrag(p, e);
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			p->m_spRowDragger->OnEndDrag(p, e);
//		}
//
//		static void MouseLeave(CGridView* p, const MouseLeaveEvent& e)
//		{
//			p->m_spRowDragger->OnLeaveDrag(p, e);
//		}
//
//	};
//	class ColDrag
//	{
//	public:
//		static void LButtonBeginDrag(CGridView* p, const LButtonBeginDragEvent& e)
//		{
//			p->m_spColDragger->OnBeginDrag(p, e);
//		}
//
//		static bool Guard_LButtonBeginDrag(CGridView* p, const LButtonBeginDragEvent& e)
//		{
//			return p->m_spColDragger->IsTarget(p, e);
//		}
//
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			p->m_spColDragger->OnDrag(p, e);
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			p->m_spColDragger->OnEndDrag(p, e);
//		}
//
//		static void MouseLeave(CGridView* p, const MouseLeaveEvent& e)
//		{
//			p->m_spColDragger->OnLeaveDrag(p, e);
//		}
//	};
//	class ItemDrag
//	{
//	public:
//		static void LButtonBeginDrag(CGridView* p, const LButtonBeginDragEvent& e)
//		{
//			p->m_spItemDragger->OnBeginDrag(p, e);
//		}
//
//		static bool Guard_LButtonBeginDrag(CGridView* p, const LButtonBeginDragEvent& e)
//		{
//			return p->m_spItemDragger->IsTarget(p, e);
//		}
//
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			p->m_spItemDragger->OnDrag(p, e);
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			p->m_spCursorer->OnLButtonUp(p, e);
//			p->m_spCeller->OnLButtonUp(p, e);
//
//			p->m_spItemDragger->OnEndDrag(p, e);
//		}
//
//		static void MouseLeave(CGridView* p, const MouseLeaveEvent& e)
//		{
//			p->m_spItemDragger->OnLeaveDrag(p, e);
//		}
//
//	};
//
//	class Edit
//	{
//	public:
//		static void MouseMove(CGridView* p, const MouseMoveEvent& e)
//		{
//			if (p->GetEditPtr()->GetClientRect().PtInRect(p->GetDirectPtr()->Pixels2Dips(e.Point))) {
//				p->GetEditPtr()->OnMouseMove(e);
//			}
//			else {
//				Normal::MouseMove(p, e);
//			}
//		}
//
//		static bool Guard_LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			return !p->GetEditPtr()->GetClientRect().PtInRect(p->GetDirectPtr()->Pixels2Dips(e.Point));
//		}
//
//		static void LButtonDown(CGridView* p, const LButtonDownEvent& e)
//		{
//			p->GetEditPtr()->OnLButtonDown(e);
//		}
//
//		static void LButtonUp(CGridView* p, const LButtonUpEvent& e)
//		{
//			if (p->GetEditPtr()->GetClientRect().PtInRect(p->GetDirectPtr()->Pixels2Dips(e.Point))) {
//				p->GetEditPtr()->OnLButtonUp(e);
//			}
//			else {
//				Normal::LButtonUp(p, e);
//			}
//		}
//
//		static bool Guard_KeyDown(CGridView* p, const KeyDownEvent& e)
//		{
//
//			if ((e.Char == VK_RETURN) && !(::GetKeyState(VK_MENU) & 0x8000) ||
//				(e.Char == VK_TAB) && !(::GetKeyState(VK_MENU) & 0x8000)) {
//				//Commit Edit
//				return true;
//			}
//			else if (e.Char == VK_ESCAPE) {
//				//CancelEdit
//				p->GetEditPtr()->CancelEdit();
//				return true;
//			}
//			else {
//				return false;
//			}
//		}
//
//		static void KeyDown(CGridView* p, const KeyDownEvent& e)
//		{
//			p->GetEditPtr()->OnKeyDown(e);
//		}
//
//		static void Char(CGridView* p, const CharEvent& e)
//		{
//			p->GetEditPtr()->OnChar(e);
//		}
//
//	};
//
//	struct Machine
//	{
//		auto operator()() const noexcept
//		{
//			using namespace sml;
//			return make_transition_table(
//			   *state<Normal> +event<LButtonDownEvent> / call(Normal::LButtonDown),
//				state<Normal> +event<LButtonDownEvent>[call(VsclDrag::Guard_LButtonDown)] / call(VsclDrag::LButtonDown) = state<VsclDrag>,
//				state<Normal> +event<LButtonDownEvent>[call(HsclDrag::Guard_LButtonDown)] / call(HsclDrag::LButtonDown) = state<HsclDrag>,
//				state<Normal> +event<LButtonDownEvent>[call(RowTrack::Guard_LButtonDown)] / call(RowTrack::LButtonDown) = state<RowTrack>,
//				state<Normal> +event<LButtonDownEvent>[call(ColTrack::Guard_LButtonDown)] / call(ColTrack::LButtonDown) = state<ColTrack>,
//
//				state<Normal> +event<LButtonUpEvent> / call(Normal::LButtonUp),
//				state<Normal> +event<LButtonClkEvent> / call(Normal::LButtonClk),
//				state<Normal> +event<LButtonSnglClkEvent> / call(Normal::LButtonSnglClk),
//				state<Normal> +event<LButtonDblClkEvent> / call(Normal::LButtonDblClk),
//				state<Normal> +event<RButtonDownEvent> / call(Normal::RButtonDown),
//				state<Normal> +event<MouseMoveEvent> / call(Normal::MouseMove),
//				state<Normal> +event<MouseLeaveEvent> / call(Normal::MouseLeave),
//				state<Normal> +event<ContextMenuEvent> / call(Normal::ContextMenu),
//				state<Normal> +event<SetCursorEvent> / call(Normal::SetCursor),
//				state<Normal> +event<KeyDownEvent> / call(Normal::KeyDown),
//				
//				state<Normal> +event<LButtonBeginDragEvent> [call(RowDrag::Guard_LButtonBeginDrag)] / call(RowDrag::LButtonBeginDrag) = state<RowDrag>,
//				state<Normal> +event<LButtonBeginDragEvent> [call(ColDrag::Guard_LButtonBeginDrag)] / call(ColDrag::LButtonBeginDrag) = state<ColDrag>,
//				state<Normal> +event<LButtonBeginDragEvent> [call(ItemDrag::Guard_LButtonBeginDrag)] / call(ItemDrag::LButtonBeginDrag) = state<ItemDrag>,
//				state<Normal> +event<BeginEditEvent> = state<Edit>, 
//
//
//				state<VsclDrag> +on_entry<_> / call(VsclDrag::OnEntry),
//				state<VsclDrag> +on_exit<_> / call(VsclDrag::OnExit),
//				state<VsclDrag> +event<LButtonUpEvent> / call(VsclDrag::LButtonUp) = state<Normal>,
//
//				state<HsclDrag> +on_entry<_> / call(HsclDrag::OnEntry),
//				state<HsclDrag> +on_exit<_> / call(HsclDrag::OnExit),
//				state<HsclDrag> +event<LButtonUpEvent> / call(HsclDrag::LButtonUp) = state<Normal>,
//
//				state<RowDrag> +event<LButtonUpEvent> / call(RowDrag::LButtonUp) = state<Normal>,
//				state<RowDrag> +event<MouseMoveEvent> / call(RowDrag::MouseMove),
//				state<RowDrag> +event<MouseLeaveEvent> / call(RowDrag::MouseLeave) = state<Normal>,
//
//				state<ColDrag> +event<LButtonUpEvent> / call(ColDrag::LButtonUp) = state<Normal>,
//				state<ColDrag> +event<MouseMoveEvent> / call(ColDrag::MouseMove),
//				state<ColDrag> +event<MouseLeaveEvent> / call(ColDrag::MouseLeave) = state<Normal>,
//
//				state<ItemDrag> +event<LButtonUpEvent> / call(ItemDrag::LButtonUp) = state<Normal>,
//				state<ItemDrag> +event<MouseMoveEvent> / call(ItemDrag::MouseMove),
//				state<ItemDrag> +event<MouseLeaveEvent> / call(ItemDrag::MouseLeave) = state<Normal>,
//
//				state<RowTrack> +event<LButtonUpEvent> / call(RowTrack::LButtonUp) = state<Normal>,
//				state<RowTrack> +event<MouseMoveEvent> / call(RowTrack::MouseMove),
//				state<RowTrack> +event<MouseLeaveEvent> / call(RowTrack::MouseLeave) = state<Normal>,
//
//				state<ColTrack> +event<LButtonUpEvent> / call(ColTrack::LButtonUp) = state<Normal>,
//				state<ColTrack> +event<MouseMoveEvent> / call(ColTrack::MouseMove),
//				state<ColTrack> +event<MouseLeaveEvent> / call(ColTrack::MouseLeave) = state<Normal>,
//
//				state<Edit> +event<LButtonDownEvent> [call(Edit::Guard_LButtonDown)] / call(Normal::LButtonDown) = state<Normal>,
//				state<Edit> +event<LButtonDownEvent> / call(Edit::LButtonDown),
//				state<Edit> +event<LButtonUpEvent> / call(Edit::LButtonUp),
//				state<Edit> +event<KeyDownEvent>[call(Edit::Guard_KeyDown)] / call(Normal::KeyDown) = state<Normal>,
//				state<Edit> +event<KeyDownEvent> / call(Edit::KeyDown)
//
//
//
//			);
//
//		}
//	};
//	std::unique_ptr<sml::sm<Machine>> m_pMachine;
//
//	Impl(CGridView* pGrid) :m_pMachine(new sml::sm<Machine>{pGrid}){}
//
//};
//
//CGridViewStateMachine::CGridViewStateMachine(CGridView* pGrid) :pImpl(new Impl(pGrid))
//{}
//
//CGridViewStateMachine::~CGridViewStateMachine() = default;
//
//void CGridViewStateMachine::ProcessEvent(const LButtonDownEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const LButtonUpEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const LButtonClkEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const LButtonSnglClkEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const LButtonDblClkEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const RButtonDownEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const MouseMoveEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const MouseLeaveEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const LButtonBeginDragEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const SetCursorEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const ContextMenuEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const KeyDownEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const CharEvent & e) { pImpl->m_pMachine->process_event(e); }
//void CGridViewStateMachine::ProcessEvent(const BeginEditEvent & e) { pImpl->m_pMachine->process_event(e); }
//
//
//	////     Start          Event             Target         Action                                       Guard
//	////*a_irow<NormalState, LButtonDownEvent, &Machine_::Action_Normal_LButtonDown>,
//	////*	a_irow<NormalState, LButtonUpEvent, &Machine_::Action_Normal_LButtonUp>,
//	////*	a_irow<NormalState, LButtonClkEvent, &Machine_::Action_Normal_LButtonClk>,
//	////*	a_irow<NormalState, LButtonSnglClkEvent, &Machine_::Action_Normal_LButtonSnglClk>,
//	////*	a_irow<NormalState, LButtonDblClkEvent, &Machine_::Action_Normal_LButtonDblClk>,
//	////*	a_irow<NormalState, RButtonDownEvent, &Machine_::Action_Normal_RButtonDown>,
//	////*	a_irow<NormalState, MouseMoveEvent, &Machine_::Action_Normal_MouseMove>,
//	////*	a_irow<NormalState, MouseLeaveEvent, &Machine_::Action_Normal_MouseLeave>,
//	////*	a_irow<NormalState, ContextMenuEvent, &Machine_::Action_Normal_ContextMenu>,
//	////*	a_irow<NormalState, SetCursorEvent, &Machine_::Action_Normal_SetCursor>,
//	////*	a_irow<NormalState, KeyDownEvent, &Machine_::Action_Normal_KeyDown>,
//	////	_row<NormalState,   BeginEditEvent,        EditState>,
//	////*	row<NormalState, LButtonDownEvent, VScrlDragState, &Machine_::Action_VScrlDrag_LButtonDown, &Machine_::Guard_VScrlDrag_LButtonDown>,
//	////*	row<NormalState, LButtonDownEvent, HScrlDragState, &Machine_::Action_HScrlDrag_LButtonDown, &Machine_::Guard_HScrlDrag_LButtonDown>,
//	////*	row<NormalState, LButtonBeginDragEvent, ColDragState, &Machine_::Action_ColDrag_LButtonBeginDrag, &Machine_::Guard_ColDrag_LButtonBeginDrag>,
//	////*	row<NormalState, LButtonBeginDragEvent, RowDragState, &Machine_::Action_RowDrag_LButtonBeginDrag, &Machine_::Guard_RowDrag_LButtonBeginDrag>,
//	////*	row<NormalState, LButtonBeginDragEvent, ItemDragState, &Machine_::Action_ItemDrag_LButtonBeginDrag, &Machine_::Guard_ItemDrag_LButtonBeginDrag>,
//	////*	row<NormalState, LButtonDownEvent, RowTrackState, &Machine_::Action_RowTrack_LButtonDown, &Machine_::Guard_RowTrack_LButtonDown>,
//	////*	row<NormalState, LButtonDownEvent, ColTrackState, &Machine_::Action_ColTrack_LButtonDown, &Machine_::Guard_ColTrack_LButtonDown>,
//	////	a_irow<EditState, MouseMoveEvent, &Machine_::Action_Edit_MouseMove>,
//	////	row<EditState,	  LButtonDownEvent,      NormalState,   &Machine_::Action_Edit_LButtonDown, &Machine_::Guard_Edit_LButtonDown>,
//	////	a_irow<EditState,     LButtonUpEvent,                       &Machine_::Action_Edit_LButtonUp>,
//	////	row<EditState,     KeyDownEvent,          NormalState,   &Machine_::Action_Edit_KeyDown,     &Machine_::Guard_Edit_KeyDown>,
//	////	a_irow<EditState,     CharEvent,                            &Machine_::Action_Edit_Char>,
//	////	_row<EditState,     KillFocusEvent,        NormalState>,
//	////*	a_irow<VScrlDragState, MouseMoveEvent, &Machine_::Action_VScrlDrag_MouseMove>,
//	////*	a_row<VScrlDragState, LButtonUpEvent, NormalState, &Machine_::Action_VScrlDrag_LButtonUp>,
//	////*	a_irow<HScrlDragState, MouseMoveEvent, &Machine_::Action_HScrlDrag_MouseMove>,
//	////*	a_row<HScrlDragState, LButtonUpEvent, NormalState, &Machine_::Action_HScrlDrag_LButtonUp>,
//	////*	a_irow<RowDragState, MouseMoveEvent, &Machine_::Action_RowDrag_MouseMove>,
//	////*	a_row<RowDragState, LButtonUpEvent, NormalState, &Machine_::Action_RowDrag_LButtonUp>,
//	////*	a_row<RowDragState, MouseLeaveEvent, NormalState, &Machine_::Action_RowDrag_MouseLeave>,
//	////	_row<RowDragState, Exception, NormalState>,
//	////*	a_irow<ColDragState, MouseMoveEvent, &Machine_::Action_ColDrag_MouseMove>,
//	////*	a_row<ColDragState, LButtonUpEvent, NormalState, &Machine_::Action_ColDrag_LButtonUp>,
//	////*	a_row<ColDragState, MouseLeaveEvent, NormalState, &Machine_::Action_ColDrag_MouseLeave>,
//	////	_row<ColDragState, Exception, NormalState>,
//	////*	a_irow<ItemDragState, MouseMoveEvent, &Machine_::Action_ItemDrag_MouseMove>,
//	////*	a_row<ItemDragState, LButtonUpEvent, NormalState, &Machine_::Action_ItemDrag_LButtonUp>,
//	////*	a_row<ItemDragState, MouseLeaveEvent, NormalState, &Machine_::Action_ItemDrag_MouseLeave>,
//	////	_row<ItemDragState, Exception, NormalState>,
//	////*	a_irow<RowTrackState, MouseMoveEvent, &Machine_::Action_RowTrack_MouseMove>,
//	////*	a_row<RowTrackState, LButtonUpEvent, NormalState, &Machine_::Action_RowTrack_LButtonUp>,
//	////*	a_row<RowTrackState, MouseLeaveEvent, NormalState, &Machine_::Action_RowTrack_MouseLeave>,
//	////	_row<RowTrackState, Exception, NormalState>,
//	////*	a_irow<ColTrackState, MouseMoveEvent, &Machine_::Action_ColTrack_MouseMove>,
//	////*	a_row<ColTrackState, LButtonUpEvent, NormalState, &Machine_::Action_ColTrack_LButtonUp>,
//	////*	a_row<ColTrackState, MouseLeaveEvent, NormalState, &Machine_::Action_ColTrack_MouseLeave>,
//	////	_row<ColTrackState, Exception, NormalState>
//
//
