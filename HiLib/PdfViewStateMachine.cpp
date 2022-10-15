#pragma once
#include "PdfViewStateMachine.h"
#include "PdfView.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

struct CPdfViewStateMachine::Machine
{
	struct Normal
	{
		class NormalPan {};
		class NormalText {};
		class NormalDebug {};

		template<class TRect, class R, class... Ts>
		auto call(R(TRect::* f)(Ts...))const
		{
			return [f](TRect* self, Ts... args) { return (self->*f)(args...); };
		}
		auto operator()() const noexcept
		{
			using namespace sml;
			return make_transition_table(
				state<NormalPan>(H) +event<LButtonDownEvent> / call(&CPdfView::NormalPan_LButtonDown),
				state<NormalPan> + event<SetCursorEvent> / call(&CPdfView::NormalPan_SetCursor),
				state<NormalPan> + event<LButtonDblClkEvent> = state<NormalText>,

				state<NormalText> +event<LButtonDownEvent> / call(&CPdfView::NormalText_LButtonDown),
				state<NormalText> + event<SetCursorEvent> / call(&CPdfView::NormalText_SetCursor),
				state<NormalText> + event<LButtonDblClkEvent> = state<NormalDebug>,

				state<NormalDebug> +event<LButtonDownEvent> / call(&CPdfView::NormalText_LButtonDown),
				state<NormalDebug> + event<SetCursorEvent> / call(&CPdfView::NormalText_SetCursor),
				state<NormalDebug> + event<LButtonDblClkEvent> = state<NormalPan>
			);
		}
	};

	struct VScrlDrag {};
	struct HScrlDrag {};
	struct Panning {};
	struct TextDrag {};
	struct Error {};

	template<class TRect, class R, class... Ts>
	auto call(R(TRect::* f)(Ts...))const
	{
		return [f](TRect* self, Ts... args) { return (self->*f)(args...); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<Normal> +event<PaintEvent> / call(&CPdfView::Normal_Paint),

			state<Normal> +event<LButtonBeginDragEvent>[call(&CPdfView::VScrlDrag_Guard_LButtonBeginDrag)] = state<VScrlDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CPdfView::HScrlDrag_Guard_LButtonBeginDrag)] = state<HScrlDrag>,	
			state<Normal> +event<LButtonBeginDragEvent>[call(&CPdfView::Panning_Guard_LButtonBeginDrag)] = state<Panning>,
			state<Normal> +event<LButtonBeginDragEvent> = state<TextDrag>,


			//state<Normal> +event<LButtonDownEvent>[call(&CPdfView::VScrlDrag_Guard_LButtonDown)] = state<VScrlDrag>,
			//state<Normal> +event<LButtonDownEvent>[call(&CPdfView::HScrlDrag_Guard_LButtonDown)] = state<HScrlDrag>,	
			//state<Normal> +event<LButtonDownEvent> = state<Panning>,
			//state<Normal> +event<LButtonDownEvent> / call(&CPdfView::Normal_LButtonDown),
			state<Normal> +event<LButtonUpEvent> / call(&CPdfView::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&CPdfView::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&CPdfView::Normal_LButtonSnglClk),
			//state<Normal> +event<LButtonDblClkEvent> / call(&CPdfView::Normal_LButtonDblClk),
			state<Normal> +event<RButtonDownEvent> / call(&CPdfView::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&CPdfView::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CPdfView::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&CPdfView::Normal_ContextMenu),
			//state<Normal> +event<SetCursorEvent> / call(&CPdfView::Normal_SetCursor),
			state<Normal> +event<KeyDownEvent> / call(&CPdfView::Normal_KeyDown),
			state<Normal> +event<CharEvent> / call(&CPdfView::Normal_Char),
			state<Normal> +event<SetFocusEvent> / call(&CPdfView::Normal_SetFocus),
			state<Normal> +event<KillFocusEvent> / call(&CPdfView::Normal_KillFocus),

			//VScrlDrag
			state<VScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CPdfView::VScrlDrag_OnEntry),
			state<VScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CPdfView::VScrlDrag_OnExit),
			//state<VScrlDrag> +on_entry<LButtonDownEvent> / call(&CPdfView::VScrlDrag_OnEntry),
			//state<VScrlDrag> +on_exit<LButtonUpEvent> / call(&CPdfView::VScrlDrag_OnExit),

			state<VScrlDrag> +event<LButtonEndDragEvent> = state<Normal>,
			//state<VScrlDrag> +event<LButtonUpEvent> = state<Normal>,

			state<VScrlDrag> +event<MouseMoveEvent> / call(&CPdfView::VScrlDrag_MouseMove),
			state<VScrlDrag> +event<SetCursorEvent> / call(&CPdfView::VScrlDrag_SetCursor),
			state<VScrlDrag> +event<PaintEvent> / call(&CPdfView::Normal_Paint),

			//HScrlDrag
			state<HScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CPdfView::HScrlDrag_OnEntry),
			state<HScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CPdfView::HScrlDrag_OnExit),
			state<HScrlDrag> +event<LButtonEndDragEvent> = state<Normal>,

			state<HScrlDrag> +event<MouseMoveEvent> / call(&CPdfView::HScrlDrag_MouseMove),
			state<HScrlDrag> +event<SetCursorEvent> / call(&CPdfView::HScrlDrag_SetCursor),
			state<HScrlDrag> +event<PaintEvent> / call(&CPdfView::Normal_Paint),

			//Panning
			state<Panning> +on_entry<LButtonBeginDragEvent> / call(&CPdfView::Panning_OnEntry),
			state<Panning> +on_exit<LButtonEndDragEvent> / call(&CPdfView::Panning_OnExit),
			state<Panning> +event<LButtonEndDragEvent> = state<Normal>,

			state<Panning> +event<MouseMoveEvent> / call(&CPdfView::Panning_MouseMove),
			state<Panning> +event<SetCursorEvent> / call(&CPdfView::Panning_SetCursor),
			state<Panning> +event<PaintEvent> / call(&CPdfView::Normal_Paint),

			//TextDrag
			state<TextDrag> +on_entry<LButtonBeginDragEvent> / call(&CPdfView::TextDrag_OnEntry),
			state<TextDrag> +on_exit<LButtonEndDragEvent> / call(&CPdfView::TextDrag_OnExit),
			state<TextDrag> +event<LButtonEndDragEvent> = state<Normal>,

			state<TextDrag> +event<MouseMoveEvent> / call(&CPdfView::TextDrag_MouseMove),
			state<TextDrag> +event<SetCursorEvent> / call(&CPdfView::TextDrag_SetCursor),
			state<TextDrag> +event<PaintEvent> / call(&CPdfView::Normal_Paint),

			//Error handler
			*state<Error> +exception<std::exception> / call(&CPdfView::Error_StdException) = state<Normal>
		);


	}
};



CPdfViewStateMachine::CPdfViewStateMachine(CPdfView* pPdfView)
	:m_pMachine(new boost::sml::sm<Machine>{ pPdfView })
{}

CPdfViewStateMachine::~CPdfViewStateMachine() = default;

bool CPdfViewStateMachine::IsStateNormalPan()const
{
	using namespace sml;
	return m_pMachine->is<decltype(state<Machine::Normal>)>(state<Machine::Normal::NormalPan>);
}
bool CPdfViewStateMachine::IsStateNormalDebug()const
{
	using namespace sml;
	return m_pMachine->is<decltype(state<Machine::Normal>)>(state<Machine::Normal::NormalDebug>);
}

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