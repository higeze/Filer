#pragma
#include "UIElement.h"
#include <memory>

//Pre-declare
class CSheet;
struct MouseEvent;

struct CMouseStateMachine
{
	CMouseStateMachine(CSheet* pSheet);
	~CMouseStateMachine();

	struct Impl;
	std::unique_ptr<Impl> pImpl;

	void LButtonDown(const LButtonDownEvent& e);
	void LButtonUp(const LButtonUpEvent& e);
	void LButtonDblClk(const LButtonDblClkEvent& e);
	void LButtonDblClkTimeExceed(const LButtonDblClkTimeExceedEvent& e);
	void MouseLeave(const MouseLeaveEvent& e);
};