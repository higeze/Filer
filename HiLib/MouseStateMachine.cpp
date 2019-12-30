#include "MouseStateMachine.h"
#include "GridView.h"
#include "DeadlineTimer.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

struct CMouseStateMachine::Machine
{
	class Normal{};
	class LButtonDowned{};
	class LButtonUpped{};
	class LButtonDblClked{};
	class LButtonDrag{};

	std::shared_ptr<CDeadlineTimer> m_pDeadlineTimer;

	Machine() :m_pDeadlineTimer(std::make_shared<CDeadlineTimer>()) {}

	void Normal_LButtonUp(CGridView* pGrid, const LButtonUpEvent& e)
	{
		pGrid->CSheet::OnLButtonClk(LButtonClkEvent(pGrid, e.Flags, MAKELPARAM(e.Point.x, e.Point.y)));
		pGrid->CSheet::OnLButtonUp(e);
	}

	void Normal_MouseLeave(CGridView* pGrid, const MouseLeaveEvent& e)
	{
		m_pDeadlineTimer->stop();
		pGrid->CSheet::OnMouseLeave(e);
	}
		
	void Normal_LButtonDown(CGridView* pGrid, const LButtonDownEvent& e)
	{
		m_pDeadlineTimer->run([pGrid, e] {
			pGrid->PostMessage(WM_LBUTTONDBLCLKTIMEXCEED, NULL, MAKELPARAM(e.Point.x, e.Point.y));
			}, std::chrono::milliseconds(::GetDoubleClickTime()));

		pGrid->CSheet::OnLButtonDown(e);
	}

	void LButtonDowned_LButtonDblClkTimeExceed(CGridView* pGrid, const LButtonDblClkTimeExceedEvent& e)
	{
		pGrid->CSheet::OnLButtonBeginDrag(LButtonBeginDragEvent(pGrid, e.Flags, MAKELPARAM(e.Point.x, e.Point.y)));
	}

	void LButtonUpped_LButtonDblClk(CGridView* pGrid, const LButtonDblClkEvent& e)
	{
		pGrid->CSheet::OnLButtonDblClk(e);
	}

	void LButtonUpped_LButtonDblClkTimeExceed(CGridView* pGrid, const LButtonDblClkTimeExceedEvent& e)
	{
		pGrid->CSheet::OnLButtonSnglClk(LButtonSnglClkEvent(pGrid, e.Flags, MAKELPARAM(e.Point.x, e.Point.y)));
		m_pDeadlineTimer->stop();
	}

	void LButtonDrag_MouseLeave(CGridView* pGrid, const MouseLeaveEvent& e)
	{
		m_pDeadlineTimer->stop();
		//m_pSheet->OnLButtonUp((MouseEventArgs)e.Args);
		pGrid->CSheet::OnMouseLeave(e);
	}

	template <class R, class... Ts>
	auto call(R(Machine::* f)(Ts...))
	{
		return [this, f](Ts... args) { return (this->*f)(args...); };
	}

	auto operator()() noexcept //Not const
	{
		using namespace sml;

		return make_transition_table(
			*state<Normal> +event<LButtonDownEvent> / call(&Machine::Normal_LButtonDown) = state<LButtonDowned>,
			state<Normal> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonDowned> +event<LButtonDblClkTimeExceedEvent> / call(&Machine::LButtonDowned_LButtonDblClkTimeExceed) = state<LButtonDrag>,
			state<LButtonDowned> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<LButtonUpped>,
			state<LButtonDowned> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonUpped> +event<LButtonDownEvent> / call(&Machine::Normal_LButtonDown) = state<LButtonDowned>,
			state<LButtonUpped> +event<LButtonDblClkEvent> / call(&Machine::LButtonUpped_LButtonDblClk) = state<LButtonDblClked>,
			state<LButtonUpped> +event<LButtonDblClkTimeExceedEvent> / call(&Machine::LButtonUpped_LButtonDblClkTimeExceed) = state<Normal>,
			state<LButtonUpped> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonDblClked> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDblClked> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonDrag> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDrag> +event<MouseLeaveEvent> / call(&Machine::LButtonDrag_MouseLeave) = state<Normal>
		);
	}
};

CMouseStateMachine::CMouseStateMachine(CGridView* pGrid)
	:m_pMachine(new boost::sml::sm<Machine>{ pGrid }){ }

CMouseStateMachine::~CMouseStateMachine() = default;

void CMouseStateMachine::process_event(const LButtonDownEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonUpEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonDblClkTimeExceedEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
