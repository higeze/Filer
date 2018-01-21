#pragma

#include <memory>

//Pre-declare
class CSheet;
struct MouseEventArgs;

struct CMouseStateMachine
{
	CMouseStateMachine();
	~CMouseStateMachine();

	struct Impl;
	std::unique_ptr<Impl> pImpl;

	void LButtonDown(CSheet * pSheet, MouseEventArgs& e);
	void LButtonUp(CSheet * pSheet, MouseEventArgs& e);
	void LButtonDblClk(CSheet * pSheet, MouseEventArgs& e);
	void LButtonDblClkTimeExceed(CSheet * pSheet, MouseEventArgs& e);
	void MouseLeave(CSheet * pSheet, MouseEventArgs& e);
};