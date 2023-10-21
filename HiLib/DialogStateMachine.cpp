#pragma once
#include "DialogStateMachine.h"
#include "D2DWDialog.h"

namespace sml = boost::sml;

struct CDialogStateMachine::Machine
{
	class Normal {};
	class Moving {};
	class LeftSizing {};
	class RightSizing {};
	class TopSizing {};
	class BottomSizing {};
	class Error {};

	template<class TRect, class R, class... Ts>
	auto call(R(TRect::* f)(Ts...))const
	{
		return [f](TRect* self, Ts... args) { return (self->*f)(args...); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<Normal> +event<LButtonBeginDragEvent>[call(&CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_LeftSizing)] / call(&CD2DWDialog::Normal_LButtonBeginDrag) = state<LeftSizing>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_RightSizing)] / call(&CD2DWDialog::Normal_LButtonBeginDrag)  = state<RightSizing>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_TopSizing)] / call(&CD2DWDialog::Normal_LButtonBeginDrag)  = state<TopSizing>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_BottomSizing)] / call(&CD2DWDialog::Normal_LButtonBeginDrag)  = state<BottomSizing>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CD2DWDialog::Guard_LButtonBeginDrag_Normal_To_Moving)] / call(&CD2DWDialog::Normal_LButtonBeginDrag)  = state<Moving>,
			state<Normal> +event<LButtonBeginDragEvent> / call(&CD2DWDialog::Normal_LButtonBeginDrag),

			state<Normal> +event<MouseMoveEvent> / call(&CD2DWDialog::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CD2DWDialog::Normal_MouseLeave),
			state<Normal> +event<SetCursorEvent> / call(&CD2DWDialog::Normal_SetCursor),
			
			//Moving
			state<Moving> +on_entry<LButtonBeginDragEvent> / call(&CD2DWDialog::Moving_OnEntry),
			state<Moving> +on_exit<_> / call(&CD2DWDialog::Moving_OnExit),
			state<Moving> +event<LButtonEndDragEvent> = state<Normal>,
			state<Moving> +event<MouseMoveEvent> / call(&CD2DWDialog::Moving_MouseMove),
			state<Moving> +event<MouseLeaveEvent> = state<Normal>,
			//Sizing
			state<LeftSizing> +on_entry<LButtonBeginDragEvent> / call(&CD2DWDialog::Sizing_OnEntry),
			state<RightSizing> +on_entry<LButtonBeginDragEvent> / call(&CD2DWDialog::Sizing_OnEntry),
			state<TopSizing> +on_entry<LButtonBeginDragEvent> / call(&CD2DWDialog::Sizing_OnEntry),
			state<BottomSizing> +on_entry<LButtonBeginDragEvent> / call(&CD2DWDialog::Sizing_OnEntry),

			state<LeftSizing> +on_exit<_> / call(&CD2DWDialog::Sizing_OnExit),
			state<RightSizing> +on_exit<_> / call(&CD2DWDialog::Sizing_OnExit),
			state<TopSizing> +on_exit<_> / call(&CD2DWDialog::Sizing_OnExit),
			state<BottomSizing> +on_exit<_> / call(&CD2DWDialog::Sizing_OnExit),

			state<LeftSizing> +event<LButtonEndDragEvent> / call(&CD2DWDialog::Normal_LButtonEndDrag) = state<Normal>,
			state<RightSizing> +event<LButtonEndDragEvent> / call(&CD2DWDialog::Normal_LButtonEndDrag) = state<Normal>,
			state<TopSizing> +event<LButtonEndDragEvent> / call(&CD2DWDialog::Normal_LButtonEndDrag) = state<Normal>,
			state<BottomSizing> +event<LButtonEndDragEvent> / call(&CD2DWDialog::Normal_LButtonEndDrag) = state<Normal>,

			state<LeftSizing> +event<MouseLeaveEvent> = state<Normal>,
			state<RightSizing> +event<MouseLeaveEvent> = state<Normal>,
			state<TopSizing> +event<MouseLeaveEvent> = state<Normal>,
			state<BottomSizing> +event<MouseLeaveEvent> = state<Normal>,

			state<LeftSizing> +event<SetCursorEvent> / call(&CD2DWDialog::LeftSizing_SetCursor),
			state<RightSizing> +event<SetCursorEvent> / call(&CD2DWDialog::RightSizing_SetCursor),
			state<TopSizing> +event<SetCursorEvent> / call(&CD2DWDialog::TopSizing_SetCursor),
			state<BottomSizing> +event<SetCursorEvent> / call(&CD2DWDialog::BottomSizing_SetCursor),

			state<LeftSizing> +event<MouseMoveEvent> / call(&CD2DWDialog::LeftSizing_MouseMove),
			state<RightSizing> +event<MouseMoveEvent> / call(&CD2DWDialog::RightSizing_MouseMove),
			state<TopSizing> +event<MouseMoveEvent> / call(&CD2DWDialog::TopSizing_MouseMove),
			state<BottomSizing> +event<MouseMoveEvent> / call(&CD2DWDialog::BottomSizing_MouseMove),
			//Error handler
			state<Error> +exception<std::exception> / call(&CD2DWDialog::Error_StdException) = state<Normal>
		);


	}
};



CDialogStateMachine::CDialogStateMachine(CD2DWDialog* pDialog)
	:m_pMachine(new boost::sml::sm<Machine>{ pDialog })
{}

CDialogStateMachine::~CDialogStateMachine() = default;

void CDialogStateMachine::process_event(const LButtonBeginDragEvent& e) { m_pMachine->process_event(e); }
void CDialogStateMachine::process_event(const LButtonEndDragEvent& e) { m_pMachine->process_event(e); }
void CDialogStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CDialogStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CDialogStateMachine::process_event(const SetCursorEvent& e) { m_pMachine->process_event(e); }
