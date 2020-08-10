#pragma once
#include "TextboxStateMachine.h"
#include "Textbox.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

struct CTextboxStateMachine::Machine
{
	class Normal {};
	class VScrlDrag {};
	class HScrlDrag {};
	class Error {};

	template<class T, class R, class... Ts>
	auto call(R(T::* f)(Ts...))const
	{
		return [f](T* self, Ts... args) {return (self->*f)(args...); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<Normal> +event<PaintEvent> / call(&D2DTextbox::Normal_Paint),

			state<Normal> +event<LButtonDownEvent>[call(&D2DTextbox::VScrlDrag_Guard_LButtonDown)] = state<VScrlDrag>,
			state<Normal> +event<LButtonDownEvent>[call(&D2DTextbox::HScrlDrag_Guard_LButtonDown)]  = state<HScrlDrag>,
			state<Normal> +event<LButtonDownEvent> / call(&D2DTextbox::Normal_LButtonDown),

			state<Normal> +event<LButtonUpEvent> / call(&D2DTextbox::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&D2DTextbox::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&D2DTextbox::Normal_LButtonSnglClk),
			state<Normal> +event<LButtonDblClkEvent> / call(&D2DTextbox::Normal_LButtonDblClk),
			state<Normal> +event<RButtonDownEvent> / call(&D2DTextbox::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&D2DTextbox::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&D2DTextbox::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&D2DTextbox::Normal_ContextMenu),
			//state<Normal> +event<SetCursorEvent>[call(&D2DTextbox::VScrl_Guard_SetCursor)] / call(&D2DTextbox::VScrl_SetCursor),
			//state<Normal> +event<SetCursorEvent>[call(&D2DTextbox::HScrl_Guard_SetCursor)] / call(&D2DTextbox::HScrl_SetCursor),
			state<Normal> +event<SetCursorEvent> / call(&D2DTextbox::Normal_SetCursor),
			state<Normal> +event<KeyDownEvent> / call(&D2DTextbox::Normal_KeyDown),
			state<Normal> +event<CharEvent> / call(&D2DTextbox::Normal_Char),
			state<Normal> +event<SetFocusEvent> / call(&D2DTextbox::Normal_SetFocus),
			state<Normal> +event<KillFocusEvent> / call(&D2DTextbox::Normal_KillFocus),

			//VScrlDrag
			state<VScrlDrag> +event<PaintEvent> / call(&D2DTextbox::Normal_Paint),
			state<VScrlDrag> +on_entry<LButtonDownEvent> / call(&D2DTextbox::VScrlDrag_OnEntry),
			state<VScrlDrag> +on_exit<_> / call(&D2DTextbox::VScrlDrag_OnExit),
			state<VScrlDrag> +event<LButtonUpEvent> = state<Normal>,
			state<VScrlDrag> +event<MouseMoveEvent> / call(&D2DTextbox::VScrlDrag_MouseMove),
			//HScrlDrag
			state<HScrlDrag> +event<PaintEvent> / call(&D2DTextbox::Normal_Paint),
			state<HScrlDrag> +on_entry<LButtonDownEvent> / call(&D2DTextbox::HScrlDrag_OnEntry),
			state<HScrlDrag> +on_exit<_> / call(&D2DTextbox::HScrlDrag_OnExit),
			state<HScrlDrag> +event<LButtonUpEvent> = state<Normal>,
			state<HScrlDrag> +event<MouseMoveEvent> / call(&D2DTextbox::HScrlDrag_MouseMove),
			//Error handler
			*state<Error> +exception<std::exception> / call(&D2DTextbox::Error_StdException) = state<Normal>
		);


	}
};



CTextboxStateMachine::CTextboxStateMachine(D2DTextbox* pTextbox)
	:m_pMachine(new boost::sml::sm<Machine>{ pTextbox })
{
}

CTextboxStateMachine::~CTextboxStateMachine() = default;

void CTextboxStateMachine::process_event(const PaintEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const LButtonDownEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const LButtonUpEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const LButtonClkEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const LButtonSnglClkEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const LButtonDblClkEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const RButtonDownEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const MouseMoveEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const MouseLeaveEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const LButtonBeginDragEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const SetCursorEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const ContextMenuEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const KeyDownEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const CharEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const SetFocusEvent & e) { m_pMachine->process_event(e); }
void CTextboxStateMachine::process_event(const KillFocusEvent & e) { m_pMachine->process_event(e); }