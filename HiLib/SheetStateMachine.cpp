#include "SheetStateMachine.h"
#include "Sheet.h"

namespace sml =boost::sml;

struct CSheetStateMachine::Machine
{
	class Normal {};
	class RowDrag {};
	class ColDrag {};
	class ItemDrag {};
	class RowTrack {};
	class ColTrack {};
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
			//Normal
			*state<Normal> +event<PaintEvent> / call(&CSheet::Normal_Paint),
			state<Normal> +event<LButtonDownEvent>[call(&CSheet::RowTrack_Guard_LButtonDown)] / call(&CSheet::RowTrack_LButtonDown) = state<RowTrack>,
			state<Normal> +event<LButtonDownEvent>[call(&CSheet::ColTrack_Guard_LButtonDown)] / call(&CSheet::ColTrack_LButtonDown) = state<ColTrack>,
			state<Normal> +event<LButtonDownEvent> / call(&CSheet::Normal_LButtonDown),

			state<Normal> +event<LButtonUpEvent> / call(&CSheet::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&CSheet::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&CSheet::Normal_LButtonSnglClk),
			state<Normal> +event<LButtonDblClkEvent> / call(&CSheet::Normal_LButtonDblClk),
			state<Normal> +event<RButtonDownEvent> / call(&CSheet::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&CSheet::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CSheet::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&CSheet::Normal_ContextMenu),
			state<Normal> +event<SetCursorEvent>[call(&CSheet::Normal_Guard_SetCursor)] / call(&CSheet::Normal_SetCursor),
			state<Normal> +event<KeyDownEvent> / call(&CSheet::Normal_KeyDown),
			state<Normal> +event<KeyTraceDownEvent> / call(&CSheet::Normal_KeyTraceDown),
			state<Normal> +event<CharEvent> / call(&CSheet::Normal_Char),
			state<Normal> +event<ImeStartCompositionEvent> / call(&CSheet::Normal_ImeStartComposition),
			state<Normal> +event<SetFocusEvent> / call(&CSheet::Normal_SetFocus),
			state<Normal> +event<KillFocusEvent> / call(&CSheet::Normal_KillFocus),

			state<Normal> +event<LButtonBeginDragEvent>[call(&CSheet::RowDrag_Guard_LButtonBeginDrag)]  = state<RowDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CSheet::ColDrag_Guard_LButtonBeginDrag)]  = state<ColDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CSheet::ItemDrag_Guard_LButtonBeginDrag)] = state<ItemDrag>,
			state<Normal> +event<LButtonBeginDragEvent> / call(&CSheet::Normal_LButtonBeginDrag),
			//
			state<RowDrag> +event<PaintEvent> / call(&CSheet::Normal_Paint),
			state<RowDrag> +on_entry<LButtonBeginDragEvent> / call(&CSheet::RowDrag_OnEntry),
			state<RowDrag> +on_exit<LButtonEndDragEvent> / call(&CSheet::RowDrag_OnExit),
			state<RowDrag> +event<LButtonEndDragEvent> = state<Normal>,
			state<RowDrag> +event<MouseMoveEvent> / call(&CSheet::RowDrag_MouseMove),
			state<RowDrag> +event<MouseLeaveEvent> / call(&CSheet::RowDrag_MouseLeave) = state<Normal>,

			state<ColDrag> +event<PaintEvent> / call(&CSheet::Normal_Paint),
			state<ColDrag> +on_entry<LButtonBeginDragEvent> / call(&CSheet::ColDrag_OnEntry),
			state<ColDrag> +on_exit<LButtonEndDragEvent> / call(&CSheet::ColDrag_OnExit),
			state<ColDrag> +event<LButtonEndDragEvent> = state<Normal>,
			state<ColDrag> +event<MouseMoveEvent> / call(&CSheet::ColDrag_MouseMove),
			state<ColDrag> +event<MouseLeaveEvent> / call(&CSheet::ColDrag_MouseLeave) = state<Normal>,

			state<ItemDrag> +event<PaintEvent> / call(&CSheet::Normal_Paint),
			state<ItemDrag> +on_entry<LButtonBeginDragEvent> / call(&CSheet::ItemDrag_OnEntry),
			state<ItemDrag> +on_exit<LButtonEndDragEvent> / call(&CSheet::ItemDrag_OnExit),
			state<ItemDrag> +event<LButtonEndDragEvent> = state<Normal>,
			state<ItemDrag> +event<MouseMoveEvent> / call(&CSheet::ItemDrag_MouseMove),
			state<ItemDrag> +event<MouseLeaveEvent> / call(&CSheet::ItemDrag_MouseLeave) = state<Normal>,

			state<RowTrack> +event<PaintEvent> / call(&CSheet::Normal_Paint),
			state<RowTrack> +event<LButtonUpEvent> / call(&CSheet::RowTrack_LButtonUp) = state<Normal>,
			state<RowTrack> +event<MouseMoveEvent> / call(&CSheet::RowTrack_MouseMove),
			state<RowTrack> +event<MouseLeaveEvent> / call(&CSheet::RowTrack_MouseLeave) = state<Normal>,

			state<ColTrack> +event<PaintEvent> / call(&CSheet::Normal_Paint),
			state<ColTrack> +event<LButtonUpEvent> / call(&CSheet::ColTrack_LButtonUp) = state<Normal>,
			state<ColTrack> +event<MouseMoveEvent> / call(&CSheet::ColTrack_MouseMove),
			state<ColTrack> +event<MouseLeaveEvent> / call(&CSheet::ColTrack_MouseLeave) = state<Normal>,
			//Error handler
			*state<Error> +exception<std::exception> / call(&CSheet::Error_StdException) = state<Normal>
		);
	}
};

CSheetStateMachine::CSheetStateMachine(CSheet* pSheet)
	:m_pMachine(new boost::sml::sm<Machine>{pSheet}){ }

CSheetStateMachine::~CSheetStateMachine() = default;

void CSheetStateMachine::process_event(const PaintEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const LButtonDownEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const LButtonUpEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const LButtonClkEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const LButtonSnglClkEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const RButtonDownEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const LButtonBeginDragEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const LButtonEndDragEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const SetCursorEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const ContextMenuEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const KeyDownEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const KeyTraceDownEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const CharEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const ImeStartCompositionEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const BeginEditEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const EndEditEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const SetFocusEvent& e) { m_pMachine->process_event(e); }
void CSheetStateMachine::process_event(const KillFocusEvent& e) { m_pMachine->process_event(e); }

