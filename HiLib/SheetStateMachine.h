#pragma once

//boost
#include <boost\statechart\state_machine.hpp>
#include <boost\statechart\simple_state.hpp>
#include <boost\statechart\event.hpp>
#include <boost\statechart\transition.hpp>
#include <boost\statechart\custom_reaction.hpp>
#include <boost\statechart\in_state_reaction.hpp>
#include <boost\mpl\list.hpp>

//local
#include "Sheet.h"

namespace sc = boost::statechart;

//Events
namespace events
{
	template<typename T>
	class MouseEvent :public sc::event<T>
	{
	public:
		CSheet* SheetPtr;
		MouseEventArgs Args;

		MouseEvent(CSheet* pSheet, MouseEventArgs e) :SheetPtr(pSheet), Args(e) {}
	};

	class LButtonDown :public MouseEvent<LButtonDown> { public: LButtonDown(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class LButtonUp :public MouseEvent<LButtonUp> { public: LButtonUp(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class LButtonSnglClk :public MouseEvent<LButtonSnglClk> { public: LButtonSnglClk(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class LButtonDblClk :public MouseEvent<LButtonDblClk> { public: LButtonDblClk(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class LButtonBeginDrag :public MouseEvent<LButtonBeginDrag> { public: LButtonBeginDrag(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class LButtonEndDrag :public MouseEvent<LButtonEndDrag> { public: LButtonEndDrag(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class MouseMove :public MouseEvent<MouseMove> { public: MouseMove(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class MouseLeave :public MouseEvent<MouseLeave> { public: MouseLeave(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
	class SetCursor :public MouseEvent<SetCursor> { public: SetCursor(CSheet* pSheet, MouseEventArgs e) :MouseEvent(pSheet, e) {} };
}

//Pre-declare initial state for state machine declaration
namespace states
{
	class Normal;
}

//State machine
class SheetStateMachine :public sc::state_machine<SheetStateMachine, states::Normal> {};

//States
namespace states
{
	class Normal;
	class RowTrack;
	class ColTrack;
	class RowDrag;
	class ColDrag;
	class ItemDrag;

	class Normal :public sc::simple_state<Normal, SheetStateMachine>
	{
	public:
		typedef boost::mpl::list<
			sc::custom_reaction<events::LButtonBeginDrag>> reactions;

		sc::result react(events::LButtonBeginDrag const& e)
		{
			if (!e.SheetPtr->Visible()) {
				return discard_event();
			}

			auto visIndexes = e.SheetPtr->Coordinates2Indexes<VisTag>(e.Args.Point);
			auto maxRow = e.SheetPtr->GetMaxIndex<RowTag, VisTag>();
			auto minRow = e.SheetPtr->GetMinIndex<RowTag, VisTag>();
			auto maxCol = e.SheetPtr->GetMaxIndex<ColTag, VisTag>();
			auto minCol = e.SheetPtr->GetMinIndex<ColTag, VisTag>();

			//Track transition
			//if ((visIndexes.first <= maxRow || e.Args.Point.x > e.SheetPtr->LastPointer<ColTag, VisTag>()->GetRight() + CBand::kResizeAreaHarfWidth) &&
			//	visIndexes.first >= minRow &&
			//	visIndexes.second <= maxCol &&
			//	visIndexes.second >= minCol) {
			//	if (visIndexes.second < minCol) {
			//		//out of left
			//		//do nothing	
			//	}
			//	else if (visIndexes.second > maxCol) {
			//		//out of right
			//		if (e.Args.Point.x < e.SheetPtr->LastPointer<ColTag, VisTag>->GetRight() + CBand::kResizeAreaHarfWidth) {
			//			return transit<ColTrack>(); //oncolumnheaderbegintrack(psheet, e, visibleindexes.second - 1);
			//		}
			//	}
			//	else if (e.Args.Point.x < (e.SheetPtr->Index2Pointer<ColTag, VisTag>(visIndexes.second)->getleft() + CBand::kResizeAreaHarfWidth)) {
			//		return transit<ColTrack>();// oncolumnheaderbegintrack(psheet, e, max(visibleindexes.second - 1, mincolumn));
			//	}
			//	else if ((e.SheetPtr->Index2Pointer<ColTag, VisTag>(visIndexes.second)->getright() - CBand::kResizeAreaHarfWidth) < e.Args.point.x) {
			//		return transit<ColTrack>();//oncolumnheaderbegintrack(psheet, e, min(visibleindexes.second, maxcolumn));
			//	}
			//	else {
			//		//do nothing
			//	}
			//}


			//Drag Transition
			if (visIndexes.first <= maxRow &&
				visIndexes.first >= minRow &&
				visIndexes.second <= maxCol &&
				visIndexes.second >= minCol) {

				if (e.SheetPtr->Index2Pointer<RowTag, VisTag>(visIndexes.first)->IsDragTrackable()) {
					return transit<states::RowDrag>();
				}
				else if (e.SheetPtr->Index2Pointer<ColTag, VisTag>(visIndexes.second)->IsDragTrackable()) {
					return transit<states::ColDrag>();
				}
				else {
					return transit<states::ItemDrag>();
				}
			}
			return discard_event();
		}
	};

	//class RowTrack :public sc::simple_state<RowTrack, SheetStateMachine> 
	//{
	//public:
	//	typedef boost::mpl::list<
	//		sc::in_state_reaction<events::MouseMove>,
	//		sc::custom_reaction<events::LButtonEndDrag>> reactions;
	//};

	class RowDrag :public sc::simple_state<RowDrag, SheetStateMachine>
	{
	public:
		typedef boost::mpl::list<
			sc::in_state_reaction<events::MouseMove>,
			sc::custom_reaction<events::LButtonEndDrag>> reactions;


	};


}
