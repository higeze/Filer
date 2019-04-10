#pragma once

struct LButtonDownEvent;
struct LButtonUpEvent;
struct MouseMoveEvent;
struct MouseLeaveEvent;

namespace d2dw
{
	class CScroll;

	struct CScrollStateMachine
	{
		CScrollStateMachine(CScroll* pScroll);
		~CScrollStateMachine();

		struct Impl;
		std::unique_ptr<Impl> pImpl;

		void LButtonDown(const LButtonDownEvent& e);
		void LButtonUp(const LButtonUpEvent& e);
		//void LButtonClk(const LButtonClkEvent& e);
		//void LButtonSnglClk(const LButtonSnglClkEvent& e);
		//void LButtonDblClk(const LButtonDblClkEvent& e);
		//void LButtonBeginDrag(const LButtonBeginDragEvent& e);
		//void RButtonDown(const RButtonDownEvent& e);
		//void ContextMenu(const ContextMenuEvent& e);
		void MouseMove(const MouseMoveEvent& e);
		void MouseLeave(const MouseLeaveEvent& e);
		//void SetCursor(const SetCursorEvent& e);
		//void KeyDown(const KeyDownEvent& e);
	};
}

