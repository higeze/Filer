#pragma

#include <memory>

//Pre-declare
class CSheet;
struct MouseEventArgs;
struct SetCursorEventArgs;
struct ContextMenuEventArgs;
struct KeyEventArgs;

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
	void ContextMenu(CSheet* pSheet, ContextMenuEventArgs& e);
	void MouseMove(CSheet * pSheet, MouseEventArgs& e);
	void MouseLeave(CSheet * pSheet, MouseEventArgs& e);
	void SetCursor(CSheet * pSheet, SetCursorEventArgs& e);
	void KeyDown(CSheet* pSheet, KeyEventArgs& e);

	void LButtonBeginDrag(CSheet * pSheet, MouseEventArgs& e);
	void LButtonEndDrag(CSheet * pSheet, MouseEventArgs& e);

};