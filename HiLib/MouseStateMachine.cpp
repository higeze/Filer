#include "MouseStateMachine.h"
#include "UIElement.h"
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

	void Normal_LButtonDown(CUIElement* pGrid, const LButtonDownEvent& e)
	{
		pGrid->OnLButtonDown(e);
	}

	void Normal_LButtonUp(CUIElement* pGrid, const LButtonUpEvent& e)
	{
		pGrid->OnLButtonClk(LButtonClkEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		pGrid->OnLButtonUp(e);
	}

	void Normal_RButtonDown(CUIElement* pGrid, const RButtonDownEvent& e)
	{
		pGrid->OnRButtonDown(e);
	}

	void Normal_MouseMove(CUIElement* pGrid, const MouseMoveEvent& e)
	{
		pGrid->OnMouseMove(e);
	}

	void Normal_MouseLeave(CUIElement* pGrid, const MouseLeaveEvent& e)
	{
		m_pDeadlineTimer->stop();
		pGrid->OnMouseLeave(e);
	}

	void Normal_MouseWheel(CUIElement* pGrid, const MouseWheelEvent& e)
	{
		pGrid->OnMouseWheel(e);
	}

	void Normal_Char(CUIElement* pGrid, const CharEvent& e)
	{
		pGrid->OnChar(e);
	}

	void Normal_KeyDown(CUIElement* pGrid, const KeyDownEvent& e)
	{
		pGrid->OnKeyDown(e);
	}

	void LButtonDowned_OnEntry(CUIElement* pGrid, const LButtonDownEvent& e)
	{
		m_ptBeginClient = e.PointInClient;
		m_pDeadlineTimer->run([pGrid, e] {
			e.WndPtr->PostMessage(RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED"), NULL, MAKELPARAM(e.PointInClient.x, e.PointInClient.y));
			}, std::chrono::milliseconds(::GetDoubleClickTime()));
	}

	void LButtonDowned_OnExit(CUIElement* pGrid)
	{
	}

	bool LButtonDowned_Guard_MouseMove(CUIElement* pGrid, const MouseMoveEvent& e)
	{
		if (m_ptBeginClient.has_value()) {
			CPoint ptBeginScreen = m_ptBeginClient.value();
			e.WndPtr->ClientToScreen(ptBeginScreen);
			auto cxdrag = ::GetSystemMetrics(SM_CXDRAG);
			auto cydrag = ::GetSystemMetrics(SM_CXDRAG);
			return std::abs(m_ptBeginClient.value().x - e.PointInClient.x) > cxdrag || std::abs(m_ptBeginClient.value().y - e.PointInClient.y) > cydrag;
			// DragDetect disable LButtonUp if return false. Do not use.
			//return ::DragDetect(e.WndPtr->m_hWnd, ptBeginScreen);
		} else {
			return false;
		}
	}

	void LButtonUpped_LButtonDblClk(CUIElement* pGrid, const LButtonDblClkEvent& e)
	{
		pGrid->OnLButtonDblClk(e);
	}

	void LButtonUpped_LButtonDblClkTimeExceed(CUIElement* pGrid, const LButtonDblClkTimeExceedEvent& e)
	{
		pGrid->OnLButtonSnglClk(LButtonSnglClkEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		m_pDeadlineTimer->stop();
	}

	void LButtonDrag_OnEntry(CUIElement* pGrid, const MouseMoveEvent& e)
	{
		pGrid->OnLButtonBeginDrag(LButtonBeginDragEvent(e.WndPtr, e.Flags, MAKELPARAM(m_ptBeginClient.value().x, m_ptBeginClient.value().y)));
		m_ptBeginClient = std::nullopt;
		e.WndPtr->SetCapture();
	}

	void LButtonDrag_OnExit(CUIElement* pGrid)
	{
		::ReleaseCapture();
	}

	void LButtonDrag_MouseLeave(CUIElement* pGrid, const MouseLeaveEvent& e)
	{
		m_pDeadlineTimer->stop();
		pGrid->OnMouseLeave(e);
	}

	bool LButtonDrag_Guard_Char(CUIElement* pGrid, const CharEvent& e)
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
			state<Normal> +event<RButtonDownEvent> / call(&Machine::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave),
			state<Normal> +event<MouseWheelEvent> / call(&Machine::Normal_MouseWheel),
			state<Normal> +event<CharEvent> / call(&Machine::Normal_Char),
			state<Normal> +event<KeyDownEvent> / call(&Machine::Normal_KeyDown),


			state<LButtonDowned> +on_entry<LButtonDownEvent> / call(&Machine::LButtonDowned_OnEntry),
			state<LButtonDowned> +on_exit<_> / call(&Machine::LButtonDowned_OnExit),
			//state<LButtonDowned> +event<LButtonDblClkTimeExceedEvent> / call(&Machine::LButtonDowned_LButtonDblClkTimeExceed) = state<LButtonDrag>,
			state<LButtonDowned> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<LButtonUpped>,
			state<LButtonDowned> +event<MouseMoveEvent> [call(&Machine::LButtonDowned_Guard_MouseMove)] = state<LButtonDrag>,
			state<LButtonDowned> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<LButtonDowned> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonUpped> +event<LButtonDownEvent> / call(&Machine::Normal_LButtonDown) = state<LButtonDowned>,
			state<LButtonUpped> +event<LButtonDblClkEvent> / call(&Machine::LButtonUpped_LButtonDblClk) = state<LButtonDblClked>,
			state<LButtonUpped> +event<LButtonDblClkTimeExceedEvent> / call(&Machine::LButtonUpped_LButtonDblClkTimeExceed) = state<Normal>,
			state<LButtonUpped> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<LButtonUpped> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonDblClked> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDblClked> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<LButtonDblClked> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,

			state<LButtonDrag> +on_entry<MouseMoveEvent> / call(&Machine::LButtonDrag_OnEntry),
			state<LButtonDrag> +on_exit<_> / call(&Machine::LButtonDrag_OnExit),
			state<LButtonDrag> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDrag> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<LButtonDrag> +event<MouseLeaveEvent> / call(&Machine::LButtonDrag_MouseLeave) = state<Normal>,
			state<LButtonDrag> +event<CharEvent> [call(&Machine::LButtonDrag_Guard_Char)] = state<Normal>,
			state<LButtonDrag> +event<CancelModeEvent> = state<Normal>,
			state<LButtonDrag> +event<CaptureChangedEvent> = state<Normal>

		);
	}
};

CMouseStateMachine::CMouseStateMachine(CUIElement* pGrid)
	:m_pMachine(new boost::sml::sm<Machine>{ pGrid }){ }

CMouseStateMachine::~CMouseStateMachine() = default;

void CMouseStateMachine::process_event(const LButtonDownEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonUpEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonDblClkTimeExceedEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const RButtonDownEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseWheelEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CharEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const KeyDownEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CancelModeEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CaptureChangedEvent& e) { m_pMachine->process_event(e); }
