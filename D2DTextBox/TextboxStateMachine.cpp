#pragma once
#include "TextboxStateMachine.h"
#include "Textbox.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

struct CTextBoxStateMachine::Machine
{
	class Normal {};
	class VScrlDrag {};
	class HScrlDrag {};
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
			*state<Normal> +event<PaintEvent> / call(&CTextBox::Normal_Paint),

			state<Normal> +event<LButtonDownEvent>[call(&CTextBox::VScrlDrag_Guard_LButtonDown)] = state<VScrlDrag>,
			state<Normal> +event<LButtonDownEvent>[call(&CTextBox::HScrlDrag_Guard_LButtonDown)] = state<HScrlDrag>,
			state<Normal> +event<LButtonDownEvent> / call(&CTextBox::Normal_LButtonDown),

			state<Normal> +event<LButtonUpEvent> / call(&CTextBox::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&CTextBox::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&CTextBox::Normal_LButtonSnglClk),
			state<Normal> +event<LButtonDblClkEvent> / call(&CTextBox::Normal_LButtonDblClk),
			state<Normal> +event<RButtonDownEvent> / call(&CTextBox::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&CTextBox::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CTextBox::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&CTextBox::Normal_ContextMenu),
			//state<Normal> +event<SetCursorEvent>[call(&D2DTextbox::VScrl_Guard_SetCursor)] / call(&D2DTextbox::VScrl_SetCursor),
			//state<Normal> +event<SetCursorEvent>[call(&D2DTextbox::HScrl_Guard_SetCursor)] / call(&D2DTextbox::HScrl_SetCursor),
			state<Normal> +event<SetCursorEvent> / call(&CTextBox::Normal_SetCursor),
			state<Normal> +event<KeyDownEvent> / call(&CTextBox::Normal_KeyDown),
			state<Normal> +event<CharEvent> / call(&CTextBox::Normal_Char),
			state<Normal> +event<SetFocusEvent> / call(&CTextBox::Normal_SetFocus),
			state<Normal> +event<KillFocusEvent> / call(&CTextBox::Normal_KillFocus),

			//VScrlDrag
			state<VScrlDrag> +event<PaintEvent> / call(&CTextBox::Normal_Paint),
			state<VScrlDrag> +on_entry<LButtonDownEvent> / call(&CTextBox::VScrlDrag_OnEntry),
			state<VScrlDrag> +on_exit<_> / call(&CTextBox::VScrlDrag_OnExit),
			state<VScrlDrag> +event<LButtonUpEvent> = state<Normal>,
			state<VScrlDrag> +event<MouseMoveEvent> / call(&CTextBox::VScrlDrag_MouseMove),
			//HScrlDrag
			state<HScrlDrag> +event<PaintEvent> / call(&CTextBox::Normal_Paint),
			state<HScrlDrag> +on_entry<LButtonDownEvent> / call(&CTextBox::HScrlDrag_OnEntry),
			state<HScrlDrag> +on_exit<_> / call(&CTextBox::HScrlDrag_OnExit),
			state<HScrlDrag> +event<LButtonUpEvent> = state<Normal>,
			state<HScrlDrag> +event<MouseMoveEvent> / call(&CTextBox::HScrlDrag_MouseMove),
			//Error handler
			*state<Error> +exception<std::exception> / call(&CTextBox::Error_StdException) = state<Normal>
		);


	}
};



CTextBoxStateMachine::CTextBoxStateMachine(CTextBox* pTextbox)
	:m_pMachine(new boost::sml::sm<Machine>{ pTextbox })
{}

CTextBoxStateMachine::~CTextBoxStateMachine() = default;

void CTextBoxStateMachine::process_event(const PaintEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const LButtonDownEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const LButtonUpEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const LButtonClkEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const LButtonSnglClkEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const RButtonDownEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const LButtonBeginDragEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const SetCursorEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const ContextMenuEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const KeyDownEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const CharEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const SetFocusEvent& e) { m_pMachine->process_event(e); }
void CTextBoxStateMachine::process_event(const KillFocusEvent& e) { m_pMachine->process_event(e); }