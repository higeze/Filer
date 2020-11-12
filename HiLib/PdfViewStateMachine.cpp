#pragma once
#include "PdfViewStateMachine.h"
#include "PdfView.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

struct CPdfViewStateMachine::Machine
{
	class Normal {};
	class VScrlDrag {};
	class HScrlDrag {};
	class Panning {};
	class Error {};

	template<class T, class R, class... Ts>
	auto call(R(T::* f)(Ts...))const
	{
		return [f](T* self, Ts... args) { return (self->*f)(args...); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<Normal> +event<PaintEvent> / call(&CPdfView::Normal_Paint),

			state<Normal> +event<LButtonBeginDragEvent>[call(&CPdfView::VScrlDrag_Guard_LButtonBeginDrag)] = state<VScrlDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CPdfView::HScrlDrag_Guard_LButtonBeginDrag)] = state<HScrlDrag>,	
			state<Normal> +event<LButtonBeginDragEvent> = state<Panning>,

			state<Normal> +event<LButtonDownEvent> / call(&CPdfView::Normal_LButtonDown),
			state<Normal> +event<LButtonUpEvent> / call(&CPdfView::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&CPdfView::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&CPdfView::Normal_LButtonSnglClk),
			state<Normal> +event<LButtonDblClkEvent> / call(&CPdfView::Normal_LButtonDblClk),
			state<Normal> +event<RButtonDownEvent> / call(&CPdfView::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&CPdfView::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CPdfView::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&CPdfView::Normal_ContextMenu),
			//state<Normal> +event<SetCursorEvent>[call(&D2DTextbox::VScrl_Guard_SetCursor)] / call(&D2DTextbox::VScrl_SetCursor),
			//state<Normal> +event<SetCursorEvent>[call(&D2DTextbox::HScrl_Guard_SetCursor)] / call(&D2DTextbox::HScrl_SetCursor),
			state<Normal> +event<SetCursorEvent> / call(&CPdfView::Normal_SetCursor),
			state<Normal> +event<KeyDownEvent> / call(&CPdfView::Normal_KeyDown),
			state<Normal> +event<CharEvent> / call(&CPdfView::Normal_Char),
			state<Normal> +event<SetFocusEvent> / call(&CPdfView::Normal_SetFocus),
			state<Normal> +event<KillFocusEvent> / call(&CPdfView::Normal_KillFocus),

			//VScrlDrag
			state<VScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CPdfView::VScrlDrag_OnEntry),
			state<VScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CPdfView::VScrlDrag_OnExit),
			state<VScrlDrag> +event<LButtonEndDragEvent> = state<Normal>,
			state<VScrlDrag> +event<MouseMoveEvent> / call(&CPdfView::VScrlDrag_MouseMove),
			state<VScrlDrag> +event<PaintEvent> / call(&CPdfView::Normal_Paint),
			//HScrlDrag
			state<HScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CPdfView::HScrlDrag_OnEntry),
			state<HScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CPdfView::HScrlDrag_OnExit),
			state<HScrlDrag> +event<LButtonEndDragEvent> = state<Normal>,
			state<HScrlDrag> +event<MouseMoveEvent> / call(&CPdfView::HScrlDrag_MouseMove),
			state<HScrlDrag> +event<PaintEvent> / call(&CPdfView::Normal_Paint),
			//Panning
			state<Panning> +on_entry<LButtonBeginDragEvent> / call(&CPdfView::Panning_OnEntry),
			state<Panning> +on_exit<LButtonEndDragEvent> / call(&CPdfView::Panning_OnExit),
			state<Panning> +event<LButtonEndDragEvent> = state<Normal>,
			state<Panning> +event<MouseMoveEvent> / call(&CPdfView::Panning_MouseMove),
			state<Panning> +event<PaintEvent> / call(&CPdfView::Normal_Paint),

			//Error handler
			*state<Error> +exception<std::exception> / call(&CPdfView::Error_StdException) = state<Normal>
		);


	}
};



CPdfViewStateMachine::CPdfViewStateMachine(CPdfView* pPdfView)
	:m_pMachine(new boost::sml::sm<Machine>{ pPdfView })
{}

CPdfViewStateMachine::~CPdfViewStateMachine() = default;

void CPdfViewStateMachine::process_event(const PaintEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const LButtonDownEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const LButtonUpEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const LButtonClkEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const LButtonSnglClkEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const RButtonDownEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const LButtonBeginDragEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const LButtonEndDragEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const SetCursorEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const ContextMenuEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const KeyDownEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const CharEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const SetFocusEvent& e) { m_pMachine->process_event(e); }
void CPdfViewStateMachine::process_event(const KillFocusEvent& e) { m_pMachine->process_event(e); }