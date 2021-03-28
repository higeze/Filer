#include "MouseStateMachine.h"
#include "D2DWWindow.h"
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

	void Normal_LButtonDown(CD2DWWindow* pGrid, const LButtonDownEvent& e)
	{
		pGrid->OnLButtonDown(e);
	}

	void Normal_LButtonUp(CD2DWWindow* pGrid, const LButtonUpEvent& e)
	{
		pGrid->OnLButtonClk(LButtonClkEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		pGrid->OnLButtonUp(e);
	}

	void Normal_RButtonDown(CD2DWWindow* pGrid, const RButtonDownEvent& e)
	{
		pGrid->OnRButtonDown(e);
	}

	void Normal_ContextMenu(CD2DWWindow* pGrid, const ContextMenuEvent& e)
	{
		pGrid->OnContextMenu(e);
	}

	void Normal_MouseMove(CD2DWWindow* pGrid, const MouseMoveEvent& e)
	{
		pGrid->OnMouseMove(e);
	}

	void Normal_MouseLeave(CD2DWWindow* pGrid, const MouseLeaveEvent& e)
	{
		m_pDeadlineTimer->stop();
		pGrid->OnMouseLeave(e);
	}

	void Normal_MouseWheel(CD2DWWindow* pGrid, const MouseWheelEvent& e)
	{
		pGrid->OnMouseWheel(e);
	}

	void Normal_Char(CD2DWWindow* pGrid, const CharEvent& e)
	{
		pGrid->OnChar(e);
	}

	void Normal_KeyDown(CD2DWWindow* pGrid, const KeyDownEvent& e)
	{
		pGrid->OnKeyDown(e);
	}

	void LButtonDowned_OnEntry(CD2DWWindow* pGrid, const LButtonDownEvent& e)
	{
		::OutputDebugString(L"LButtonDowned_OnEntry\r\n");
		m_ptBeginClient = e.PointInClient;
		m_pDeadlineTimer->run([pGrid, e] {
			e.WndPtr->PostMessage(RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED"), NULL, MAKELPARAM(e.PointInClient.x, e.PointInClient.y));
			}, std::chrono::milliseconds(::GetDoubleClickTime()));
	}

	void LButtonDowned_OnExit(CD2DWWindow* pGrid)
	{
	}

	bool LButtonDowned_Guard_MouseMove(CD2DWWindow* pGrid, const MouseMoveEvent& e)
	{
		if (m_ptBeginClient.has_value()) {
			auto cxdrag = ::GetSystemMetrics(SM_CXDRAG);
			auto cydrag = ::GetSystemMetrics(SM_CXDRAG);
			return std::abs(m_ptBeginClient.value().x - e.PointInClient.x) > cxdrag || std::abs(m_ptBeginClient.value().y - e.PointInClient.y) > cydrag;
			// Do not use DragDetect. Since it disables LButtonUp if return false.
		} else {
			return false;
		}
	}

	void LButtonUpped_LButtonDblClk(CD2DWWindow* pGrid, const LButtonDblClkEvent& e)
	{
		pGrid->OnLButtonDblClk(e);
	}

	void LButtonUpped_LButtonDblClkTimeExceed(CD2DWWindow* pGrid, const LButtonDblClkTimeExceedEvent& e)
	{
		pGrid->OnLButtonSnglClk(LButtonSnglClkEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		m_pDeadlineTimer->stop();
	}

	void LButtonDrag_OnEntry(CD2DWWindow* pGrid, const MouseMoveEvent& e)
	{
		::OutputDebugString(L"LButtonDrag_OnEntry\r\n");
		e.WndPtr->SetCapture();

		//auto iter = std::find_if(e.WndPtr->GetChildControlPtrs().cbegin(), e.WndPtr->GetChildControlPtrs().cend(),
		//	[&](const std::shared_ptr<CD2DWControl>& x) {
		//		return x->GetIsEnabled().get() && x->GetRectInWnd().PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(m_ptBeginClient.value()));
		//	});
		//if (iter != e.WndPtr->GetChildControlPtrs().cend()) {
		//	e.WndPtr->SetCapturedControlPtr(*iter);
		//}

		pGrid->OnLButtonBeginDrag(LButtonBeginDragEvent(e.WndPtr, e.Flags, MAKELPARAM(m_ptBeginClient.value().x, m_ptBeginClient.value().y)));
		m_ptBeginClient = std::nullopt;
	}

	void LButtonDrag_OnExitByUp(CD2DWWindow* pGrid, const LButtonUpEvent& e)
	{
		::OutputDebugString(L"LButtonDrag_OnExit\r\n");
		::ReleaseCapture();

		pGrid->OnLButtonEndDrag(LButtonEndDragEvent(e.WndPtr, e.Flags, MAKELPARAM(e.PointInClient.x, e.PointInClient.y)));
		//pGrid->ReleaseCapturedControlPtr();
	}

	void LButtonDrag_OnExitByChar(CD2DWWindow* pGrid, const CharEvent& e)
	{
		::OutputDebugString(L"LButtonDrag_OnExit\r\n");
		::ReleaseCapture();

		auto pt = e.WndPtr->GetCursorPosInClient();
		pGrid->OnLButtonEndDrag(LButtonEndDragEvent(e.WndPtr, MK_LBUTTON, MAKELPARAM(pt.x, pt.y)));
		//pGrid->ReleaseCapturedControlPtr();
	}


	//void LButtonDrag_CaptureChanged(CUIElement* pGrid, const CaptureChangedEvent& e)
	//{
	//	pGrid->OnCaptureChanged(e);
	//}

	void LButtonDrag_MouseLeave(CD2DWWindow* pGrid, const MouseLeaveEvent& e)
	{
		m_pDeadlineTimer->stop();
		pGrid->OnMouseLeave(e);
	}

	bool LButtonDrag_Guard_Char(CD2DWWindow* pGrid, const CharEvent& e)
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
			state<Normal> +on_entry<_> / [](CD2DWWindow* pGrid) { ::OutputDebugString(L"Normal_OnEntry\r\n"); },

			*state<Normal> +event<LButtonDownEvent> / call(&Machine::Normal_LButtonDown) = state<LButtonDowned>,
			state<Normal> +event<RButtonDownEvent> / call(&Machine::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave),
			state<Normal> +event<MouseWheelEvent> / call(&Machine::Normal_MouseWheel),
			state<Normal> +event<CharEvent> / call(&Machine::Normal_Char),
			state<Normal> +event<KeyDownEvent> / call(&Machine::Normal_KeyDown),
			state<Normal> +event<ContextMenuEvent> / call(&Machine::Normal_ContextMenu),

			state<LButtonDowned> +on_entry<LButtonDownEvent> / call(&Machine::LButtonDowned_OnEntry),
			state<LButtonDowned> +on_exit<_> / call(&Machine::LButtonDowned_OnExit),
			state<LButtonDowned> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<LButtonUpped>,
			state<LButtonDowned> +event<MouseMoveEvent> [call(&Machine::LButtonDowned_Guard_MouseMove)] = state<LButtonDrag>,
			state<LButtonDowned> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<LButtonDowned> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,
			state<LButtonDowned> +event<MouseWheelEvent> / call(&Machine::Normal_MouseWheel) = state<Normal>,
			state<LButtonDowned> +event<CharEvent> / call(&Machine::Normal_Char),
			state<LButtonDowned> +event<KeyDownEvent> / call(&Machine::Normal_KeyDown),


			state<LButtonUpped> +event<LButtonDownEvent> / call(&Machine::Normal_LButtonDown) = state<LButtonDowned>,
			state<LButtonUpped> +event<LButtonDblClkEvent> / call(&Machine::LButtonUpped_LButtonDblClk) = state<LButtonDblClked>,
			state<LButtonUpped> +event<LButtonDblClkTimeExceedEvent> / call(&Machine::LButtonUpped_LButtonDblClkTimeExceed) = state<Normal>,
			state<LButtonUpped> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<LButtonUpped> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,
			state<LButtonUpped> +event<MouseWheelEvent> / call(&Machine::Normal_MouseWheel) = state<Normal>,
			state<LButtonUpped> +event<CharEvent> / call(&Machine::Normal_Char),
			state<LButtonUpped> +event<KeyDownEvent> / call(&Machine::Normal_KeyDown),
			state<LButtonUpped> +event<ContextMenuEvent> / call(&Machine::Normal_ContextMenu),


			state<LButtonDblClked> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDblClked> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			state<LButtonDblClked> +event<MouseLeaveEvent> / call(&Machine::Normal_MouseLeave) = state<Normal>,
			state<LButtonDblClked> +event<MouseWheelEvent> / call(&Machine::Normal_MouseWheel) = state<Normal>,
			state<LButtonDblClked> +event<CharEvent> / call(&Machine::Normal_Char),
			state<LButtonDblClked> +event<KeyDownEvent> / call(&Machine::Normal_KeyDown),


			state<LButtonDrag> +on_entry<MouseMoveEvent> / call(&Machine::LButtonDrag_OnEntry),
			state<LButtonDrag> +on_exit<LButtonUpEvent> / call(&Machine::LButtonDrag_OnExitByUp),
			state<LButtonDrag> +on_exit<CharEvent> / call(&Machine::LButtonDrag_OnExitByChar),

			state<LButtonDrag> +event<LButtonUpEvent> / call(&Machine::Normal_LButtonUp) = state<Normal>,
			state<LButtonDrag> +event<MouseMoveEvent> / call(&Machine::Normal_MouseMove),
			//state<LButtonDrag> +event<MouseLeaveEvent> / call(&Machine::LButtonDrag_MouseLeave) = state<Normal>,
			state<LButtonDrag> +event<CharEvent> [call(&Machine::LButtonDrag_Guard_Char)] = state<Normal>,
			state<LButtonDrag> +event<CharEvent> / call(&Machine::Normal_Char),
			state<LButtonDrag> +event<KeyDownEvent> / call(&Machine::Normal_KeyDown),
			state<LButtonDrag> +event<CancelModeEvent> = state<Normal>//,
			//state<LButtonDrag> +event<CaptureChangedEvent> / call(&Machine::LButtonDrag_CaptureChanged)

		);
	}
};

CMouseStateMachine::CMouseStateMachine(CD2DWWindow* pGrid)
	:m_pMachine(new boost::sml::sm<Machine>{ pGrid }){ }

CMouseStateMachine::~CMouseStateMachine() = default;

void CMouseStateMachine::process_event(const LButtonDownEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonUpEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const LButtonDblClkTimeExceedEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const RButtonDownEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const ContextMenuEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseMoveEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseLeaveEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const MouseWheelEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CharEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const KeyDownEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CancelModeEvent& e) { m_pMachine->process_event(e); }
void CMouseStateMachine::process_event(const CaptureChangedEvent& e) { m_pMachine->process_event(e); }
