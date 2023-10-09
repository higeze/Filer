#pragma once
#include "GridViewStateMachine.h"
#include "GridView.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

struct CGridStateMachine::Machine
{
	class Normal {};
	class VScrlDrag {};
	class HScrlDrag {};
	class RowDrag {};
	class ColDrag {};
	class ItemDrag {};
	class RowTrack {};
	class ColTrack {};
	class Edit {};
	class Error {};

	template<class TRect, class R, class... Ts>
	auto call(R(TRect::* f)(Ts...))const
	{
		return [f](TRect* self, Ts... args) {return (self->*f)(args...); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<Normal> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<Normal> +on_entry<_> / []() { ::OutputDebugStringA("Normal OnEntry\r\n"); },
			state<Normal> +on_exit<_> / []() { ::OutputDebugStringA("Normal OnExit\r\n"); },
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::VScrlDrag_Guard_LButtonDown)] / call(&CGridView::VScrlDrag_LButtonDown) = state<VScrlDrag>,
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::HScrlDrag_Guard_LButtonDown)] / call(&CGridView::HScrlDrag_LButtonDown) = state<HScrlDrag>,
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::RowTrack_Guard_LButtonDown)] / call(&CGridView::RowTrack_LButtonDown) = state<RowTrack>,
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::ColTrack_Guard_LButtonDown)] / call(&CGridView::ColTrack_LButtonDown) = state<ColTrack>,
			state<Normal> +event<LButtonDownEvent> / call(&CGridView::Normal_LButtonDown),

			state<Normal> +event<LButtonUpEvent> / call(&CGridView::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&CGridView::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&CGridView::Normal_LButtonSnglClk),
			state<Normal> +event<LButtonDblClkEvent> / call(&CGridView::Normal_LButtonDblClk),
			state<Normal> +event<RButtonDownEvent> / call(&CGridView::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&CGridView::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CGridView::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&CGridView::Normal_ContextMenu),
			state<Normal> +event<SetCursorEvent>[call(&CGridView::Normal_Guard_SetCursor)] / call(&CGridView::Normal_SetCursor),
			state<Normal> +event<KeyDownEvent> / call(&CGridView::Normal_KeyDown),
			state<Normal> +event<KeyTraceDownEvent> / call(&CGridView::Normal_KeyTraceDown),
			state<Normal> +event<CharEvent> / call(&CGridView::Normal_Char),
			state<Normal> +event<ImeStartCompositionEvent> / call(&CGridView::Normal_ImeStartComposition),
			state<Normal> +event<SetFocusEvent> / call(&CGridView::Normal_SetFocus),
			state<Normal> +event<KillFocusEvent> / call(&CGridView::Normal_KillFocus),

			state<Normal> +event<LButtonBeginDragEvent>[call(&CGridView::RowDrag_Guard_LButtonBeginDrag)]  = state<RowDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CGridView::ColDrag_Guard_LButtonBeginDrag)]  = state<ColDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CGridView::ItemDrag_Guard_LButtonBeginDrag)]  = state<ItemDrag>,
			state<Normal> +event<LButtonBeginDragEvent> / call(&CGridView::Normal_LButtonBeginDrag),
		//	state<Normal> +event<EndEditEvent> / call(&CGridView::Edit_EndEdit),

			state<Normal> +event<BeginEditEvent> = state<Edit>,
			//VScrlDrag
			state<VScrlDrag> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<VScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CGridView::VScrlDrag_OnEntry),
			state<VScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CGridView::VScrlDrag_OnExit),
			state<VScrlDrag> +event<LButtonEndDragEvent> = state<Normal>,
			state<VScrlDrag> +event<MouseMoveEvent> / call(&CGridView::VScrlDrag_MouseMove),
			//HScrlDrag
			state<HScrlDrag> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<HScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CGridView::HScrlDrag_OnEntry),
			state<HScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CGridView::HScrlDrag_OnExit),
			state<HScrlDrag> +event<LButtonEndDragEvent>  = state<Normal>,
			state<HScrlDrag> +event<MouseMoveEvent> / call(&CGridView::HScrlDrag_MouseMove),
			//RowDrag
			state<RowDrag> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<RowDrag> +on_entry<LButtonBeginDragEvent> / call(&CGridView::RowDrag_OnEntry),
			state<RowDrag> +on_exit<LButtonEndDragEvent> / call(&CGridView::RowDrag_OnExit),
			state<RowDrag> +event<LButtonEndDragEvent>  = state<Normal>,
			state<RowDrag> +event<MouseMoveEvent> / call(&CGridView::RowDrag_MouseMove),
			state<RowDrag> +event<MouseLeaveEvent> / call(&CGridView::RowDrag_MouseLeave) = state<Normal>,
			//ColDrag
			state<ColDrag> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<ColDrag> +on_entry<LButtonBeginDragEvent> / call(&CGridView::ColDrag_OnEntry),
			state<ColDrag> +on_exit<LButtonEndDragEvent> / call(&CGridView::ColDrag_OnExit),
			state<ColDrag> +event<LButtonEndDragEvent>  = state<Normal>,
			state<ColDrag> +event<MouseMoveEvent> / call(&CGridView::ColDrag_MouseMove),
			state<ColDrag> +event<MouseLeaveEvent> / call(&CGridView::ColDrag_MouseLeave) = state<Normal>,
			//ItemDrag
			state<ItemDrag> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<ItemDrag> +on_entry<LButtonBeginDragEvent> / call(&CGridView::ItemDrag_OnEntry),
			state<ItemDrag> +on_exit<LButtonEndDragEvent> / call(&CGridView::ItemDrag_OnExit),
			state<ItemDrag> +event<LButtonEndDragEvent>  = state<Normal>,
			state<ItemDrag> +event<MouseMoveEvent> / call(&CGridView::ItemDrag_MouseMove),
			state<ItemDrag> +event<MouseLeaveEvent> / call(&CGridView::ItemDrag_MouseLeave) = state<Normal>,
			//RowTrack
			state<RowTrack> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<RowTrack> +event<LButtonUpEvent> / call(&CGridView::RowTrack_LButtonUp) = state<Normal>,
			state<RowTrack> +event<MouseMoveEvent> / call(&CGridView::RowTrack_MouseMove),
			state<RowTrack> +event<MouseLeaveEvent> / call(&CGridView::RowTrack_MouseLeave) = state<Normal>,
			//ColTrack
			state<ColTrack> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<ColTrack> +event<LButtonUpEvent> / call(&CGridView::ColTrack_LButtonUp) = state<Normal>,
			state<ColTrack> +event<MouseMoveEvent> / call(&CGridView::ColTrack_MouseMove),
			state<ColTrack> +event<MouseLeaveEvent> / call(&CGridView::ColTrack_MouseLeave) = state<Normal>,
			//Edit
			state<Edit> +on_entry<BeginEditEvent> / call(&CGridView::Edit_OnEntry),
			state<Edit> +on_exit<_> / call(&CGridView::Edit_OnExit),
			state<Edit> +event<BeginEditEvent> = state<Edit>,
			state<Edit> +event<PaintEvent> / call(&CGridView::Normal_Paint),
			state<Edit> +event<LButtonDownEvent>[call(&CGridView::Edit_Guard_LButtonDown)] / call(&CGridView::Normal_LButtonDown) = state<Normal>,
			state<Edit> +event<LButtonDownEvent> / call(&CGridView::Edit_LButtonDown),
			state<Edit> +event<LButtonUpEvent> / call(&CGridView::Edit_LButtonUp),
			state<Edit> +event<LButtonBeginDragEvent> / call(&CGridView::Edit_LButtonBeginDrag),
			state<Edit> +event<LButtonEndDragEvent> / call(&CGridView::Edit_LButtonEndDrag),
			state<Edit> +event<MouseMoveEvent> / call(&CGridView::Edit_MouseMove),
			state<Edit> +event<KeyDownEvent>[call(&CGridView::Edit_Guard_KeyDown_ToNormal_Tab)] / call(&CGridView::Edit_KeyDown_Tab) = state<Normal>,
			state<Edit> +event<KeyDownEvent>[call(&CGridView::Edit_Guard_KeyDown_ToNormal)] = state<Normal>,
			state<Edit> +event<KeyDownEvent> / call(&CGridView::Edit_KeyDown),
			state<Edit> +event<CharEvent> / call(&CGridView::Edit_Char),
			state<Edit> +event<KillFocusEvent> = state<Normal>,
			state<Edit> +event<EndEditEvent>/* / call(&CGridView::Edit_EndEdit)*/ = state<Normal>,
			//Error handler
			*state<Error> +exception<std::exception> / call(&CGridView::Error_StdException) = state<Normal>
		);
			

	}
};



CGridStateMachine::CGridStateMachine(CGridView* pGrid)
	://m_logger(),
	//m_pMachine(new boost::sml::sm<Machine ,sml::logger<my_logger>>{ m_logger, pGrid, static_cast<CSheet*>(pGrid) })
	m_pMachine(new boost::sml::sm<Machine, sml::process_queue<std::queue>>{ pGrid, static_cast<CSheet*>(pGrid) })
	//m_pMachine(new boost::sml::sm<Machine>{ pGrid, static_cast<CSheet*>(pGrid)})
{
}

CGridStateMachine::~CGridStateMachine() = default;

void CGridStateMachine::process_event(const PaintEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const LButtonDownEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const LButtonUpEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const LButtonClkEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const LButtonSnglClkEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const LButtonDblClkEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const RButtonDownEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const MouseMoveEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const MouseLeaveEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const LButtonBeginDragEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const LButtonEndDragEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const SetCursorEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const ContextMenuEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const KeyDownEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const KeyTraceDownEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const CharEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const ImeStartCompositionEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const BeginEditEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const EndEditEvent& e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const SetFocusEvent & e) { m_pMachine->process_event(e); }
void CGridStateMachine::process_event(const KillFocusEvent & e) { m_pMachine->process_event(e); }