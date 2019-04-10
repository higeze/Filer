#include "Scroll.h"
#include "GridView.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>

namespace d2dw
{

	//struct CScroll::StateMachine :boost::msm::front::state_machine_def<CScroll::StateMachine>
	//{
	//	//Event
	//	struct Exception {};

	//	//State
	//	struct NormalState :boost::msm::front::state<> {};
	//	struct DragState :boost::msm::front::state<> {};

	//	//Machine

	//	struct Machine_ :state_machine_def<CScroll::StateMachine::Machine_>
	//	{
	//		FLOAT m_startDrag = 0.f;
	//		//Normal
	//		template<class Event>
	//		void Action_Normal_LButtonDown(Event const & e)
	//		{
	//			m_startDrag = e.Direct.Pixels2DipsY(e.Point.y);
	//		}

	//		template<class Event>
	//		bool Guard_Normal_LButtonDown(Event const & e)
	//		{
	//			return base->GetThumbRect().PtInRect(e.Direct.Pixels2Dips(e.Point));
	//		}

	//		template<class Event>
	//		void Action_Drag_LButtonUp(Event const & e)
	//		{
	//			m_startDrag = 0.f;
	//		}

	//		template<class Event>
	//		void Action_Drag_MouseMove(Event const & e)
	//		{
	//			base->SetScrollPos(base->GetScrollPos() + (e.Direct.Pixels2DipsY(e.Point.y) - m_startDrag) * (base->GetScrollRange().second - base->GetScrollRange().first) / base->GetRect().Height());
	//			m_startDrag = e.Direct.Pixels2DipsY(e.Point.y);
	//		}

	//		struct transition_table :boost::mpl::vector <
	//			//     Start      Event             Target       Action                                Guard
	//			row<NormalState, LButtonDownEvent, DragState, &Machine_::Action_Normal_LButtonDown, &Machine_::Guard_Normal_LButtonDown>,
	//			_row<NormalState, Exception, NormalState>,

	//			a_row<DragState, LButtonUpEvent, NormalState, &Machine_::Action_Drag_LButtonUp>,
	//			a_irow<DragState, MouseMoveEvent, &Machine_::Action_Drag_MouseMove>,
	//			_row<DragState, Exception, NormalState>
	//		>
	//		{
	//		};
	//		using initial_state = NormalState;

	//		template <class FSM, class Event>
	//		void no_transition(Event const& e, FSM&, int state) {}

	//		template <class FSM, class Event>
	//		void exception_caught(Event const& ev, FSM& fsm, std::exception& ex)
	//		{
	//			base->ResetStateMachine();
	//		}

	//		CScroll* base;
	//		Machine_(CScroll* pScroll) :base(pScroll) {}

	//	};

	//	using Machine = boost::msm::back::state_machine<Machine_>;
	//	Machine m_machine;
	//	StateMachine(CScroll* pScroll) :m_machine(pScroll) {}
	//};


	CScroll::CScroll(CGridView* pGrid) :CUIElement(), m_pGrid(pGrid)//, m_pStateMachine(std::make_unique<CScroll::StateMachine>(this))
	{
//		m_pStateMachine->m_machine.start();
	}

	CScroll::~CScroll() = default;

	void CScroll::SetScrollPos(const FLOAT pos)
	{
		FLOAT newPos = (std::max)((std::min)(pos, m_range.second - m_page), m_range.first);
		if (m_pos != newPos) {
			m_pos = newPos;
			OnPropertyChanged(L"pos");
		}
	}

	//void CScroll::ResetStateMachine() { m_pStateMachine = std::make_unique<StateMachine>(this); }

	void CScroll::OnPropertyChanged(const wchar_t* name)
	{
		m_pGrid->PostUpdate(Updates::Row);

	}


	void CScroll::OnPaint(const PaintEvent& e)
	{
		if (!GetVisible())return;
		//Draw background
		e.Direct.FillSolidRectangle(m_backgroundFill, GetRect());
		//Draw thumb
		e.Direct.FillSolidRectangle(m_foregroundFill, GetThumbRect());
	}

	//void CScroll::OnLButtonDown(const LButtonDownEvent& e)
	//{
	//	m_pStateMachine->m_machine.process_event(e);
	//}
	//void CScroll::OnLButtonUp(const LButtonUpEvent& e)
	//{
	//	m_pStateMachine->m_machine.process_event(e);
	//}
	//void CScroll::OnMouseMove(const MouseMoveEvent& e)
	//{
	//	m_pStateMachine->m_machine.process_event(e);
	//}

	//void CScroll::OnMouseWheel(const MouseWheelEvent& e)
	//{
	//}
}

