#include "MouseStateMachine.h"
#include "GridView.h"
#include "DeadlineTimer.h"
#include "MyPoint.h"
#include <optional>
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
	std::optional<CPoint> m_ptBeginClient = std::nullopt;

	Machine() :m_pDeadlineTimer(std::make_shared<CDeadlineTimer>()) {}

	void Normal_LButtonDown(CGridView* pGrid, const LButtonDownEvent& e)
	{
		pGrid->CSheet::OnLButtonDown(e);
	}

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

	void LButtonDowned_OnEntry(CGridView* pGrid, const LButtonDownEvent& e)
	{
		m_ptBeginClient = e.Point;
		m_pDeadlineTimer->run([pGrid, e] {
			pGrid->PostMessage(WM_LBUTTONDBLCLKTIMEXCEED, NULL, MAKELPARAM(e.Point.x, e.Point.y));
			}, std::chrono::milliseconds(::GetDoubleClickTime()));
	}

	void LButtonDowned_OnExit(CGridView* pGrid)
	{
	}

	bool LButtonDowned_Guard_MouseMove(CGridView* pGrid, const MouseMoveEvent& e)
	{
		if (m_ptBeginClient.has_value()) {
			CPoint ptBeginScreen = m_ptBeginClient.value();
			e.WndPtr->ClientToScreen(ptBeginScreen);
			auto cxdrag = ::GetSystemMetrics(SM_CXDRAG);
			auto cydrag = ::GetSystemMetrics(SM_CXDRAG);
			return std::abs(m_ptBeginClient.value().x - e.Point.x) > cxdrag || std::abs(m_ptBeginClient.value().y - e.Point.y) > cydrag;
			// DragDetect disable LButtonUp if return false. Do not use.
			//return ::DragDetect(e.WndPtr->m_hWnd, ptBeginScreen);
		} else {
			return false;
		}
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

	void LButtonDrag_OnEntry(CGridView* pGrid, const MouseMoveEvent& e)
	{
		pGrid->CSheet::OnLButtonBeginDrag(LButtonBeginDragEvent(pGrid, e.Flags, MAKELPARAM(m_ptBeginClient.value().x, m_ptBeginClient.value().y)));
		m_ptBeginClient = std::nullopt;
		pGrid->SetCapture();
	}

	void LButtonDrag_OnExit(CGridView* pGrid)
	{
		::ReleaseCapture();
	}

	void LButtonDrag_MouseLeave(CGridView* pGrid, const MouseLeaveEvent& e)
	{
		m_pDeadlineTimer->stop();
		pGrid->CSheet::OnMouseLeave(e);
	}

	bool LButtonDrag_Guard_Char(CGridView* pGrid, const CharEvent& e)
	{
		return e.Char == VK_ESCAPE;
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

			state<LButtonDowned> +on_entry<LButtonDownEvent> / call(&Machine::LButtonDowned_OnEntry),
			state<LButtonDowned> +on_exit<_> / call(&Machine::LButtonDowned_OnExit),
			//state<LButtonDowned> +event<LButtonDblClkTimeExceedEvent> / call(&Machine::LButtonDowned_LButtonDblClkTimeExceed) = state<LButtonDrag>,
			state<LButtonDowned> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<LButtonUpped>,
			state<LButtonDowned> +event<MouseMoveEvent> [call(&Machine::LButtonDowned_Guard_MouseMove)] = state<LButtonDrag>,
			state<LButtonDowned> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonUpped> +event<LButtonDownEvent> / call(&Machine::Normal_LButtonDown) = state<LButtonDowned>,
			state<LButtonUpped> +event<LButtonDblClkEvent> / call(&Machine::LButtonUpped_LButtonDblClk) = state<LButtonDblClked>,
			state<LButtonUpped> +event<LButtonDblClkTimeExceedEvent> / call(&Machine::LButtonUpped_LButtonDblClkTimeExceed) = state<Normal>,
			state<LButtonUpped> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonDblClked> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDblClked> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonDrag> +on_entry<MouseMoveEvent> / call(&Machine::LButtonDrag_OnEntry),
			state<LButtonDrag> +on_exit<_> / call(&Machine::LButtonDrag_OnExit),
			state<LButtonDrag> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDrag> +event<MouseLeaveEvent> / call(&Machine::LButtonDrag_MouseLeave) = state<Normal>,
			state<LButtonDrag> +event<CharEvent> [call(&Machine::LButtonDrag_Guard_Char)] = state<Normal>,
			state<LButtonDrag> +event<CancelModeEvent> = state<Normal>,
			state<LButtonDrag> +event<CaptureChangedEvent> = state<Normal>

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
void CMouseStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CharEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CancelModeEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CaptureChangedEvent& e) { m_pMachine->process_event(e); }
