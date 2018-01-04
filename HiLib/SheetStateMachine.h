#pragma

#include <memory>

//Pre-declare
class CSheet;
struct MouseEventArgs;
struct SetCursorEventArgs;

struct CSheetStateMachine
{
	CSheetStateMachine();
	~CSheetStateMachine();

	struct Impl;
	std::unique_ptr<Impl> pImpl;

	void LButtonDown(CSheet * pSheet, MouseEventArgs& e);
	void LButtonUp(CSheet * pSheet, MouseEventArgs& e);
	void LButtonSnglClk(CSheet * pSheet, MouseEventArgs& e);
	void LButtonDblClk(CSheet * pSheet, MouseEventArgs& e);
	void RButtonDown(CSheet * pSheet, MouseEventArgs& e);
	void MouseMove(CSheet * pSheet, MouseEventArgs& e);
	void MouseLeave(CSheet * pSheet, MouseEventArgs& e);
	void LButtonBeginDrag(CSheet * pSheet, MouseEventArgs& e);
	void LButtonEndDrag(CSheet * pSheet, MouseEventArgs& e);
	void SetCursor(CSheet * pSheet, SetCursorEventArgs& e);

};