#pragma once
#include "ImageViewStateMachine.h"
#include "ImageView.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

struct CImageViewStateMachine::Machine
{
	struct Normal
	{
		class NormalPan {};
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
				state<NormalPan>(H) +event<LButtonDownEvent> / call(&CImageView::NormalPan_LButtonDown),
				state<NormalPan> + event<SetCursorEvent> / call(&CImageView::NormalPan_SetCursor),
				state<NormalPan> + event<LButtonDblClkEvent> = state<NormalDebug>,

				state<NormalDebug> +event<LButtonDownEvent> / call(&CImageView::NormalPan_LButtonDown),
				state<NormalDebug> + event<SetCursorEvent> / call(&CImageView::NormalPan_SetCursor),
				state<NormalDebug> + event<LButtonDblClkEvent> = state<NormalPan>
			);
		}
	};

	struct VScrlDrag {};
	struct HScrlDrag {};
	struct Panning {};
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
			*state<Normal> +event<PaintEvent> / call(&CImageView::Normal_Paint),

			state<Normal> +event<LButtonBeginDragEvent>[call(&CImageView::VScrlDrag_Guard_LButtonBeginDrag)] = state<VScrlDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CImageView::HScrlDrag_Guard_LButtonBeginDrag)] = state<HScrlDrag>,	
			state<Normal> +event<LButtonBeginDragEvent>[call(&CImageView::Panning_Guard_LButtonBeginDrag)] = state<Panning>,
			state<Normal> +event<LButtonBeginDragEvent> = state<Panning>,

			state<Normal> +event<LButtonUpEvent> / call(&CImageView::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&CImageView::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&CImageView::Normal_LButtonSnglClk),
			state<Normal> +event<RButtonDownEvent> / call(&CImageView::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&CImageView::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CImageView::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&CImageView::Normal_ContextMenu),
			state<Normal> +event<KeyDownEvent> / call(&CImageView::Normal_KeyDown),
			state<Normal> +event<CharEvent> / call(&CImageView::Normal_Char),
			state<Normal> +event<SetFocusEvent> / call(&CImageView::Normal_SetFocus),
			state<Normal> +event<KillFocusEvent> / call(&CImageView::Normal_KillFocus),

			//VScrlDrag
			state<VScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CImageView::VScrlDrag_OnEntry),
			state<VScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CImageView::VScrlDrag_OnExit),
			state<VScrlDrag> +event<LButtonEndDragEvent> = state<Normal>,

			state<VScrlDrag> +event<MouseMoveEvent> / call(&CImageView::VScrlDrag_MouseMove),
			state<VScrlDrag> +event<SetCursorEvent> / call(&CImageView::VScrlDrag_SetCursor),
			state<VScrlDrag> +event<PaintEvent> / call(&CImageView::Normal_Paint),

			//HScrlDrag
			state<HScrlDrag> +on_entry<LButtonBeginDragEvent> / call(&CImageView::HScrlDrag_OnEntry),
			state<HScrlDrag> +on_exit<LButtonEndDragEvent> / call(&CImageView::HScrlDrag_OnExit),
			state<HScrlDrag> +event<LButtonEndDragEvent> = state<Normal>,

			state<HScrlDrag> +event<MouseMoveEvent> / call(&CImageView::HScrlDrag_MouseMove),
			state<HScrlDrag> +event<SetCursorEvent> / call(&CImageView::HScrlDrag_SetCursor),
			state<HScrlDrag> +event<PaintEvent> / call(&CImageView::Normal_Paint),

			//Panning
			state<Panning> +on_entry<LButtonBeginDragEvent> / call(&CImageView::Panning_OnEntry),
			state<Panning> +on_exit<LButtonEndDragEvent> / call(&CImageView::Panning_OnExit),
			state<Panning> +event<LButtonEndDragEvent> = state<Normal>,

			state<Panning> +event<MouseMoveEvent> / call(&CImageView::Panning_MouseMove),
			state<Panning> +event<SetCursorEvent> / call(&CImageView::Panning_SetCursor),
			state<Panning> +event<PaintEvent> / call(&CImageView::Normal_Paint),

			//Error handler
			*state<Error> +exception<std::exception> / call(&CImageView::Error_StdException) = state<Normal>
		);


	}
};

CImageViewStateMachine::CImageViewStateMachine(CImageView* pImageView)
	:m_pMachine(new boost::sml::sm<Machine>{ pImageView })
{}

CImageViewStateMachine::~CImageViewStateMachine() = default;

bool CImageViewStateMachine::IsStateNormalPan()const
{
	using namespace sml;
	return m_pMachine->is<decltype(state<Machine::Normal>)>(state<Machine::Normal::NormalPan>);
}
bool CImageViewStateMachine::IsStateNormalDebug()const
{
	using namespace sml;
	return m_pMachine->is<decltype(state<Machine::Normal>)>(state<Machine::Normal::NormalDebug>);
}

void CImageViewStateMachine::process_event(const PaintEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const LButtonDownEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const LButtonUpEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const LButtonClkEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const LButtonSnglClkEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const RButtonDownEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const LButtonBeginDragEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const LButtonEndDragEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const SetCursorEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const ContextMenuEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const KeyDownEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const CharEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const SetFocusEvent& e) { m_pMachine->process_event(e); }
void CImageViewStateMachine::process_event(const KillFocusEvent& e) { m_pMachine->process_event(e); }